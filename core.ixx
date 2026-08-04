//#include "ankerl/unordered_dense.h"
export module tecs:core;

#include "configs_macro.h"

import std;
import typelist;
import configs;
import mpl;

import :misc;
import :maths;
import :threads;

using uint_t = unsigned int;
using byte_t = unsigned char;
using ushort_t = unsigned short;

using size_t = std::size_t;
using xstring = std::string;
template<typename T, size_t N> using xarray = std::array<T, N>;
//template<typename tkey, typename tvalue> using xmap = ankerl::unordered_dense::map<tkey, tvalue>;

namespace tecs
{
	export using id_t = unsigned long long;

	//64 bit layout: [ARCHETYPE_BITS | UNUSED | VERSION_BITS | ID_BITS]
	constexpr inline size_t ENTITY_BITS = 64;
	constexpr inline size_t ID_BITS = configs::ID_BITS;
	constexpr inline size_t VERSION_BITS = configs::VERSION_BITS;
	constexpr inline size_t FLAG_BITS = 32 - configs::ARCHETYPE_BITS;
	constexpr inline size_t ARCHETYPE_BITS = configs::ARCHETYPE_BITS;

	constexpr inline size_t ID_MAX = (1ull << ID_BITS) - 1;

	const id_t MASK_ID		= (1ull << ID_BITS) - 1;
	const id_t MASK_VER		= (( 1ull << VERSION_BITS) - 1) << ID_BITS;
	const id_t VER_ONE		= 1ull << ID_BITS;
	const id_t MASK_ARCHE	= ~((1ull << (ENTITY_BITS - ARCHETYPE_BITS)) - 1);

	export const id_t ID_NULL = ID_MAX;

	export inline uint_t get_id(id_t e) { return e & MASK_ID; }
	export inline id_t set_id(id_t e, uint_t newid) { return e - get_id(e) + newid; }
	export inline uint_t get_version(id_t e) { return (e & MASK_VER) >> ID_BITS; }
	export inline id_t set_version(id_t e, id_t v) { return (e & ~MASK_VER) + (v << ID_BITS); }
	export inline id_t incr_version(id_t e) { return e + VER_ONE; }
	export inline ushort_t get_arche(id_t e) { return (e & MASK_ARCHE) >> (ENTITY_BITS - ARCHETYPE_BITS); }
	export inline id_t set_arche(id_t e, id_t arche) { return (e & ~MASK_ARCHE) + (arche << (ENTITY_BITS - ARCHETYPE_BITS)); }

	
	export struct entity_t
	{
	private:
		uint_t arche	: ARCHETYPE_BITS;
		uint_t flag		: 32 - ARCHETYPE_BITS;
		uint_t ver		: VERSION_BITS;
		uint_t id		: ID_BITS;

	public:
		static entity_t null;

	public:
		entity_t(){}
		entity_t(uint_t id_, uint_t version_, uint_t flag_, uint_t arche_)
		{
			id = id_; ver = version_; flag = flag_; arche = arche_;
		}
		//entity_t& operator=(entity_t&& ent) = default;

		uint_t get_id() { return id; }
		uint_t get_version() { return ver; }
		uint_t get_flag() { return flag; }
		uint_t get_arche() { return arche; }

		void set_id(uint_t id_) { id = id_; }
		void inc_id() { ++id; }
		void set_version(uint_t ver_) { ver = ver_; }
		void inc_version() { ++ver; }
		void set_flag(uint_t flag_) { flag = flag_; }
		void set_arche(uint_t arche_) { arche = arche_; }

		template<uint_t ChunkSize>
		inline std::pair<uint_t, uint_t> to_chunk()
		{
			return { id / ChunkSize, id & (ChunkSize - 1) };
		}

		bool valid() { return id != ID_NULL; }
		void recycle() { ver = 0; flag = 0; }

		void print() { std::cout << "id " << id << " ver " << ver << " arche " << arche << std::endl; }
	};
	entity_t entity_t::null = { ID_NULL, 0, 0, 0 };

	export template<size_t ArcheId>
	class id_generator_t
	{
	private:
		entity_t current;
		std::queue<entity_t> free;

	public:
		id_generator_t()
		{
			current = entity_t{ 0,0,0,ArcheId };
		}

		entity_t get()
		{
			if (!free.empty())
			{
				auto entity = free.front();
				free.pop();
				return  entity;
			}
			else
			{
				if (current.get_id() >= ID_MAX)
				{
					//throw
					return entity_t::null;
				}
				auto entity = current;
				current.inc_id();
				return entity;
			}
			
		}

		void recycle(entity_t e)
		{
			e.recycle();
			free.push(e);
		}
	};


	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args>
	struct archetype_storage_t
	{
		using this_type = archetype_storage_t<Args...>;
		static constexpr size_t this_id = misc::type_id_archetype<this_type>;
		
		static constexpr size_t COUNT_PER_CHUNK = 
			maths::round_power_2(configs::ARCHETYPE_CHUNK_LENGTH / (misc::data_size_of_t<Args...>::value + sizeof(entity_t) + sizeof(ushort_t)));

		static constexpr size_t CHUNKS_MAX_COUNT = ID_MAX / COUNT_PER_CHUNK;

		enum { map_component_archetype = misc::map_component_archetype<this_id, mpl::typelist<Args...>>::value };

		template<typename T>
		struct column_t
		{
			using container_t = xarray<T, COUNT_PER_CHUNK>;
			container_t values{};

			T& operator[](uint_t index) { return values[index]; }
			const T& operator[](uint_t index) const { return values[index]; }
			
			container_t::iterator begin() noexcept { return values.begin(); }

			container_t::const_iterator end() const noexcept { return values.end(); }

			
		};
		//using container_t = typename column_t<T>::container_t;

		struct chunk_t : column_t<Args>...
		{
			column_t<entity_t> entities;
			xarray< ushort_t, COUNT_PER_CHUNK> indexies;
			uint_t length = 0;
		public:
			template<typename T> auto& column() { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			template<typename T> const auto& column() const { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			template<typename T>
			void get(uint_t array_index, T&& arg)
			{
				auto& container = column<mpl::traits::remove_qualifier_t<T>>();
				arg = container[array_index];
			}

			template<typename T>
			void get(uint_t array_index, const T&& arg) const
			{
				const auto& container = column<mpl::traits::remove_qualifier_t<T>>();
				arg = container[array_index];
			}

			template<typename T>
			void set(uint_t array_index, T&& arg)
			{
				auto& container = column<mpl::traits::remove_qualifier_t<T>>();
				container[array_index] = std::forward<T>(arg);
			}
		};

		struct sparse_set_t
		{
		private:
			xarray<chunk_t*, CHUNKS_MAX_COUNT> chunks{}; 
			

		public:
			inline chunk_t* get_chunk(uint_t chunk_id) 
			{
				chunk_t* chunk_ptr = chunks[chunk_id];
				if (nullptr == chunk_ptr)
				{
					spinlock_t spin;
					spin.lock();
					if (nullptr == chunk_ptr)
					{
						chunk_ptr = new chunk_t{};
						chunks[chunk_id] = chunk_ptr;
					}
					spin.unlock();
				}
				return chunk_ptr;
			}

		};

	private:
		static sparse_set_t* sparse_set_ptr;
		static id_generator_t<this_id> id_generator;

		static sparse_set_t* sparse_set()
		{
			if (sparse_set_ptr == nullptr)
			{
				spinlock_t spin;
				spin.lock();
				if (sparse_set_ptr == nullptr)
				{
					sparse_set_ptr = new sparse_set_t();
				}
				spin.unlock();
			}
			return sparse_set_ptr;
		}

		static bool allocated() { return sparse_set_ptr != nullptr; }

	public:
		static void name() { std::cout << misc::type_name_id<this_type>() << std::endl; }

		template<typename... Ts>
		static entity_t create(Ts&&...args) 
		{
			entity_t ent = id_generator.get();
			auto [ chunk_id, index] = ent.to_chunk< COUNT_PER_CHUNK>();
			sparse_set_t* sparse = sparse_set();
			chunk_t* chunk = sparse->get_chunk(chunk_id);

			chunk->indexies[index] = chunk->length;
			chunk->entities[chunk->length] = std::move(ent);
			(chunk->set<Ts>(chunk->length, std::forward<Ts>(args)), ...);

			return ent;
		}

		template<typename... Ts>
		static void get(entity_t ent, Ts&&... args)
		{
			auto [chunk_id, index] = ent.to_chunk< COUNT_PER_CHUNK>();
			sparse_set_t* sparse = sparse_set();
			chunk_t* chunk = sparse->get_chunk(chunk_id);
			((chunk->get<Ts>(index, std::forward<Ts>(args))), ...);
		}
	};
	template<typename... Args> typename archetype_storage_t<Args...>::sparse_set_t* archetype_storage_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> id_generator_t< archetype_storage_t<Args...>::this_id>  archetype_storage_t<Args...>::id_generator{};

	template<typename... Args>
	struct archetype_define_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<Args...>::type>::type;

		static constexpr auto apply()
		{
			constexpr auto storage = apply_impl(
				misc::constexpr_array_to_index_sequence < 
					misc::sort_index_sequence(unpack_list_t{}) > (
						std::make_index_sequence<mpl::length<unpack_list_t>::value>{}));

			return storage;
		}

		template<size_t... I>
		static constexpr auto apply_impl(std::index_sequence<I...>)
		{
			using type = archetype_storage_t<mpl::at<I, unpack_list_t>::type...>;
			return type{};
		}

	public:
		using type = decltype(apply());
	};

	export template<typename... Args>
	using archetype_t = archetype_define_t<Args...>::type;

	/// <summary>
	/// forward declare for archetype_defines and query.
	/// put it here so query don't need to import archetype_defines directly.
	/// </summary>
	/// <typeparam name="ID"></typeparam>
	export template<std::size_t ID> struct archetype_of_id_t;
}




