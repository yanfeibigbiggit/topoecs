//#include "ankerl/unordered_dense.h"
export module tecs.basic:core;

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

		uint_t get_id() const { return id; }
		uint_t get_version() const { return ver; }
		uint_t get_flag() const { return flag; }
		uint_t get_arche() const { return arche; }

		void set_id(uint_t id_) { id = id_; }
		void inc_id() { ++id; }
		void set_version(uint_t ver_) { ver = ver_; }
		void inc_version() { ++ver; }
		void set_flag(uint_t flag_) { flag = flag_; }
		void set_arche(uint_t arche_) { arche = arche_; }

		bool same_id_version(entity_t other)
		{
			if(other.id != id) { return false; }
			if (other.ver != ver) { return false; }
			return true;
		}

		bool valid() const { return id != ID_NULL; }
		void recycle() { ver = 0; flag = 0; }

		template<uint_t ChunkSize>
		inline uint_t chunk_id() const { return id / ChunkSize; }
		template<uint_t ChunkSize>
		inline uint_t sparse_id() const { return id & (ChunkSize - 1); }

		//xstring fmt() { return std::format("id:{:07d} version:{:07d} arche:{:07d}", (unsigned int)id, (unsigned int)ver, (unsigned int)arche); }
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

		bool full()
		{
			return current.valid();
		}
	};

	enum archetype_state_t
	{
		read,
		write,
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
	export template<typename... Args>
	struct sparse_set_t
	{
		static constexpr size_t SPARSE_INDEX_NULL = std::numeric_limits<uint_t>::max();

		static constexpr size_t ROW_SIZE = (misc::data_size_of_t<Args...>::value + sizeof(entity_t) + sizeof(uint_t));

		static constexpr size_t decide_element_per_chunk()
		{
			size_t count = maths::round_power_2(configs::ARCHETYPE_CHUNK_ALLOC_LENGTH / ROW_SIZE);
			count = count < 1 ? 1 : count;
			return count;
		}
		static constexpr size_t COUNT_PER_CHUNK = decide_element_per_chunk();
		////////////////////////////////////////////////////////////////////////////////////////////////
		// 
		////////////////////////////////////////////////////////////////////////////////////////////////
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
		////////////////////////////////////////////////////////////////////////////////////////////////
		// 
		////////////////////////////////////////////////////////////////////////////////////////////////
		struct chunk_t : column_t<Args>...
		{
			using indexies_t = xarray<uint_t, COUNT_PER_CHUNK>;

		private:
			indexies_t indexies = utility::make_array<uint_t, COUNT_PER_CHUNK>(SPARSE_INDEX_NULL);
			uint_t length = 0;

		public:
			uint_t get_length() { return length; }

		private:
			template<typename T> auto& column() { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			template<typename T> const auto& column() const { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

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
			inline T& get_component(uint_t dense_index)
			{
				auto& container = column<T>();
				return container[dense_index];
			}
			template<typename T>
			inline const T& get_component(uint_t dense_index) const
			{
				const auto& container = column<T>();
				return container[dense_index];
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
			inline bool set_components(entity_t ent, Ts&&...args)
			{
				spinlock_t spin;
				spin.lock();

				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{
					dense_index = length;
					indexies[sparse_index] = length;
					length++;
				}
				else
				{
					entity_t& target_ent = get_component<entity_t>(dense_index);
					//must be same entity
					if (!target_ent.same_id_version(ent))
					{ return false; }
				}
				((set_component<Ts>(dense_index, std::forward<Ts>(args))), ...);
				//set at last, in case Ts... contain entity_t(not be allowed)
				set_component<entity_t>(dense_index, std::move(ent));

				spin.unlock();
				return true;
			}

			bool remove(entity_t ent)
			{
				spinlock_t spin;
				spin.lock();

				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				//has been deleted or not been alloc
				if(SPARSE_INDEX_NULL == dense_index) [[unlikely]]
				{ return false; }
				entity_t& target_ent = get_component<entity_t>(dense_index);
				//must be same entity
				if (!target_ent.same_id_version(ent))
				{ return false; }
				entity_t& last_ent = get_component<entity_t>(length - 1);
				uint_t last_sparse_index = last_ent.sparse_id< COUNT_PER_CHUNK>();
				((set_component<Args>(dense_index, std::forward<Args>(get_component<Args>(length - 1)))), ...);
				indexies[last_sparse_index] = dense_index;
				indexies[sparse_index] = SPARSE_INDEX_NULL;
				length--;

				spin.unlock();
				return true;
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


		////////////////////////////////////////////////////////////////////////////////////////////////
		// begin sparse_set_t main declare
		////////////////////////////////////////////////////////////////////////////////////////////////

		using chunk_ptrs_t = xvector<chunk_t*>;
		using indexies_t = xvector<uint_t>;

	private:
		chunk_ptrs_t chunks;
		indexies_t indexies;
		uint_t length = 0;

	public:
		sparse_set_t()
		{
			chunks = chunk_ptrs_t(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT);
			indexies = indexies_t(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT, SPARSE_INDEX_NULL);
		}

		uint_t try_dense_id(uint_t chunk_id, bool larger_then_resize)
		{
			if (larger_then_resize && chunk_id >= indexies.size()) [[unlikely]]//full then realloc sparse indexies
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

		inline chunk_t* try_get_chunk(uint_t chunk_id)
		{
			uint_t dense_id = try_dense_id(chunk_id, true);
			chunk_t* chunk_ptr = SPARSE_INDEX_NULL == dense_id ? nullptr : chunks[dense_id];
			if (nullptr == chunk_ptr) [[unlikely]]
			{
				spinlock_t spin;
				spin.lock();
				chunk_ptr = SPARSE_INDEX_NULL == dense_id ? nullptr : chunks[dense_id];
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

		inline chunk_t* get_chunk(uint_t chunk_id)
		{
			uint_t dense_id = try_dense_id(chunk_id, false);
			if (SPARSE_INDEX_NULL == dense_id) [[unlikely]]
			{ return nullptr; }
			return chunks[dense_id];
		}

		template<typename... Ts>
		inline void get_components(entity_t ent, Ts&&...args)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			if (nullptr == chunk) [[unlikely]]
			{ return; }
			chunk->get_components(ent, std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		inline bool set_components(entity_t ent, Ts&&...args)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = try_get_chunk(chunk_id);
			bool ok = chunk->set_components(ent, std::forward<Ts>(args)...);
			return ok;
		}

		bool remove(entity_t ent)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			if(nullptr == chunk) [[unlikely]]
			{ return false; }
			bool ok = chunk->remove(ent);
			return ok;
		}

		chunk_ptrs_t::iterator begin() noexcept { return chunks.begin(); }

		chunk_ptrs_t::const_iterator end() const noexcept { return chunks.begin() + length; }
	};

	export template<typename... Args>
	struct archetype_storage_t
	{
		using this_type = archetype_storage_t<Args...>;
		static constexpr size_t this_id = misc::type_id_archetype<this_type>;
		enum { map_component_archetype = misc::map_component_archetype<this_id, mpl::typelist<Args...>>::value };

		using storage_t = sparse_set_t<Args...>;

	private:
		static storage_t* sparse_set_ptr;
		static id_generator_t<this_id> id_generator;

		static storage_t* try_sparse_set()
		{
			if (sparse_set_ptr == nullptr)
			{
				spinlock_t spin;
				spin.lock();
				if (sparse_set_ptr == nullptr)
				{
					sparse_set_ptr = new storage_t();
				}
				spin.unlock();
			}
			return sparse_set_ptr;
		}

		static bool allocated() { return sparse_set_ptr != nullptr; }

		template<typename Lambda, typename... Ts>
		static void each_impl(Lambda&& lambda, mpl::typelist<Ts...>)
		{
			static_assert(!mpl::has_raw_reference_of_v<mpl::typelist<Ts...>, entity_t>, "forbidden reference of entity_t");
			if(nullptr == sparse_set_ptr) [[unlikely]]
			{ return; }
			for (auto chunk : *sparse_set_ptr)
			{
				chunk->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
			}
		}

	public:
		static void name() { std::cout << misc::type_name_id<this_type>() << std::endl; }

		bool full() 
		{
			return id_generator.full();
		}

		template<typename... Ts>
		static entity_t create(Ts&&...args) 
		{
			entity_t ent = id_generator.get();
			storage_t* sparse_ptr = try_sparse_set();
			sparse_ptr->set_components(ent, std::forward<Ts>(args)...);

			return ent;
		}

		static void remove(entity_t ent)
		{
			if(nullptr == sparse_set_ptr) [[unlikely]]
			{ return; }
			sparse_set_ptr->remove(ent);
		}

		template<typename... Ts>
		static bool get(entity_t ent, Ts&&... args)
		{
			auto chunk_id = ent.chunk_id< storage_t::COUNT_PER_CHUNK>();
			if(nullptr == sparse_set_ptr) [[unlikely]]
			{ return false; }
			sparse_set_ptr->get_components(ent, std::forward<Ts>(args)...);
			return true;
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
	template<typename... Args> typename archetype_storage_t<Args...>::storage_t* archetype_storage_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> id_generator_t< archetype_storage_t<Args...>::this_id>  archetype_storage_t<Args...>::id_generator{};

	template<typename... Args>
	struct archetype_define_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_t<entity_t, Args...>>;

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


}




