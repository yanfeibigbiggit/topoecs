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
import :utility;

using int_t = int;
using uint_t = unsigned int;
using short_t = short;
using ushort_t = unsigned short;

using size_t = std::size_t;
using xstring = std::string;
template<typename T, size_t N> using xarray = std::array<T, N>;
template<typename T> using xvector = std::vector<T>;
//template<typename tkey, typename tvalue> using xmap = ankerl::unordered_dense::map<tkey, tvalue>;

namespace tecs
{
	using id_t = unsigned long long;

	//64 bit layout: [ARCHETYPE_BITS | UNUSED | VERSION_BITS | ID_BITS]
	constexpr inline size_t ENTITY_BITS = 64;
	constexpr inline size_t ID_BITS = configs::ID_BITS;
	constexpr inline size_t VERSION_BITS = configs::VERSION_BITS;
	constexpr inline size_t FLAG_BITS = 32 - configs::ARCHETYPE_BITS;
	constexpr inline size_t ARCHETYPE_BITS = configs::ARCHETYPE_BITS;

	constexpr inline size_t ID_MAX_COUNT = (1ull << ID_BITS);
	constexpr inline size_t ID_MAX = ID_MAX_COUNT - 1;

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

		bool valid() { return id != ID_NULL; }
		void recycle() { ver = 0; flag = 0; }

		template<uint_t ChunkSize>
		inline uint_t chunk_id() { return id / ChunkSize; }
		template<uint_t ChunkSize>
		inline uint_t sparse_id() { return id & (ChunkSize - 1); }

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


	export enum e_archelayout
	{
		sparse_set,
		singleton
	};

	template<e_archelayout Layout>
	struct archepolicy_t
	{
		const e_archelayout layout = Layout;
	};
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args>
	struct archetype_storage_t
	{
		using this_type = archetype_storage_t<Args...>;
		static constexpr size_t this_id = misc::type_id_archetype<this_type>;
		enum { map_component_archetype = misc::map_component_archetype<this_id, mpl::typelist<Args...>>::value };

		static constexpr size_t SPARSE_INDEX_NULL = std::numeric_limits<uint_t>::max();

		static constexpr size_t ROW_SIZE = (misc::data_size_of_t<Args...>::value + sizeof(entity_t) + sizeof(uint_t));

		static constexpr size_t decide_element_per_chunk()
		{
			size_t count = maths::round_power_2(configs::ARCHETYPE_CHUNK_ALLOC_LENGTH / ROW_SIZE);
			count = count < 1 ? 1 : count;
			return count;
		}
		static constexpr size_t COUNT_PER_CHUNK = decide_element_per_chunk();

		template<typename T>
		struct column_t
		{
			using elements_t = xarray<T, COUNT_PER_CHUNK>;
			elements_t values{};

			T& operator[](uint_t index) { return values[index]; }
			const T& operator[](uint_t index) const { return values[index]; }
			
			elements_t::iterator begin() noexcept { return values.begin(); }

			elements_t::const_iterator end() const noexcept { return values.end(); }
		};

		struct chunk_t : column_t<Args>...
		{
			using indexies_t = xarray<uint_t, COUNT_PER_CHUNK>;

		private:
			indexies_t indexies = utility::make_array<uint_t, COUNT_PER_CHUNK>(SPARSE_INDEX_NULL);
			uint_t length = 0;

		public:
			template<typename T> auto& column() { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			template<typename T> const auto& column() const { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			uint_t get_length() { return length; }

		private:
			template<typename T>
			inline void get_component(uint_t dense_index, T&& arg)
			{
				auto& container = column<T>();
				arg = container[dense_index];
			}

			template<typename T>
			inline void get_component(uint_t dense_index, const T&& arg) const
			{
				const auto& container = column<T>();
				arg = container[dense_index];
			}

			template<typename T>
			inline void set_component(uint_t dense_index, T&& arg)
			{
				auto& container = column<T>();
				container[dense_index] = std::forward<T>(arg);
			}

		public:
			template<typename... Ts>
			inline bool get_components(entity_t ent, Ts&&...args)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{ return false; }

				((get_component<Ts>(dense_index, std::forward<Ts>(args))), ...);
				return true;
			}

			template<typename... Ts>
			inline bool get_components(entity_t ent, const Ts&&... args) const
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{ return false; }

				((get_component<Ts>(dense_index, std::forward<Ts>(args))), ...);
				return true;
			}

			template<typename... Ts>
			inline void set_components(entity_t ent, Ts&&...args)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{
					dense_index = length;
					indexies[sparse_index] = length;
					length++;
				}
				((set_component<Ts>(dense_index, std::forward<Ts>(args))), ...);
				//set at last, in case Ts... contain entity_t(not be allowed)
				set_component<entity_t>(dense_index, std::move(ent));
			}

			template<typename Lambda, typename... Ts>
			inline void each(Lambda&& lambda)
			{
				for (uint_t i = 0; i < length; i++)
				{
					lambda(std::forward<Ts>(column<Ts>()[i])...);
				}
			}
		};

		struct sparse_set_t
		{
			using chunk_ptrs_t = xvector<chunk_t*>;
			using indexies_t = xvector<uint_t>;

		private:
			chunk_ptrs_t chunks;
			indexies_t indexies;
			uint_t length = 0;

			uint_t try_dense_id(uint_t chunk_id)
			{
				if (chunk_id >= indexies.size())//full then realloc sparse indexies
				{
					spinlock_t spin;
					spin.lock();
					if (chunk_id >= indexies.size())//double guard
					{
						auto newsize = (uint_t)(chunk_id * 1.4);
						newsize = newsize >= ID_MAX ? ID_MAX - 1 : newsize;
						indexies.resize(newsize, SPARSE_INDEX_NULL);
						chunks.resize(newsize);
					}
					spin.unlock();
					return SPARSE_INDEX_NULL;
				}
				uint_t dense_id = indexies[chunk_id];
				return dense_id;
			}

		public:
			sparse_set_t()
			{
				chunks = chunk_ptrs_t(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT);
				indexies = indexies_t(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT);
			}

			inline chunk_t* get_chunk(uint_t chunk_id)
			{
				uint_t dense_id = try_dense_id(chunk_id);
				chunk_t* chunk_ptr = SPARSE_INDEX_NULL == dense_id ? nullptr : chunks[dense_id];
				if (nullptr == chunk_ptr)
				{
					spinlock_t spin;
					spin.lock();
					if (nullptr == chunk_ptr)
					{
						chunk_ptr = new chunk_t{};
						indexies[chunk_id] = length;
						chunks[length] = chunk_ptr;
						length++;
					}
					spin.unlock();
				}
				return chunk_ptr;
			}

			chunk_ptrs_t::iterator begin() noexcept { return chunks.begin(); }

			chunk_ptrs_t::const_iterator end() const noexcept { return chunks.begin() + length; }
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

		template<typename Lambda, typename... Ts>
		static void each_impl(Lambda&& lambda, mpl::typelist<Ts...>)
		{
			sparse_set_t* sparse_ptr = sparse_set();
			for (auto chunk : *sparse_ptr)
			{
				chunk->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
			}
		}

	public:
		static void name() { std::cout << misc::type_name_id<this_type>() << std::endl; }

		template<typename... Ts>
		static entity_t create(Ts&&...args) 
		{
			entity_t ent = id_generator.get();
			auto chunk_id = ent.chunk_id< COUNT_PER_CHUNK>();
			sparse_set_t* sparse_ptr = sparse_set();
			chunk_t* chunk = sparse_ptr->get_chunk(chunk_id);
			chunk->set_components(ent, std::forward<Ts>(args)...);

			return ent;
		}

		template<typename... Ts>
		static void get(entity_t ent, Ts&&... args)
		{
			auto chunk_id = ent.chunk_id< COUNT_PER_CHUNK>();
			sparse_set_t* sparse_ptr = sparse_set();
			chunk_t* chunk = sparse_ptr->get_chunk(chunk_id);
			chunk->get_components(ent, std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		static void each(void(*func)(Ts...))
		{
			each_impl(func, mpl::typelist<Ts...>{});
		}

		template<typename Lambda>
		static void each(Lambda&& lambda)
		{
			using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
			each_impl(std::forward<Lambda>(lambda), args_type{});
		}

	};
	template<typename... Args> typename archetype_storage_t<Args...>::sparse_set_t* archetype_storage_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> id_generator_t< archetype_storage_t<Args...>::this_id>  archetype_storage_t<Args...>::id_generator{};

	template<typename... Args>
	struct archetype_define_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<entity_t, Args...>::type>::type;

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
	/// </summary>
	/// <typeparam name="ID"></typeparam>
	export template<std::size_t ID> struct archetype_of_id_t;
}




