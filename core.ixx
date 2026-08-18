export module tecs.basic:core;

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
	//64 bit layout: [ARCHETYPE_BITS | UNUSED | VERSION_BITS | ID_BITS]
	constexpr inline size_t ENTITY_BITS = 64;
	constexpr inline size_t ID_BITS = configs::ID_BITS;
	constexpr inline size_t VERSION_BITS = configs::VERSION_BITS;
	constexpr inline size_t FLAG_BITS = 32 - configs::ARCHETYPE_BITS;
	constexpr inline size_t ARCHETYPE_BITS = configs::ARCHETYPE_BITS;

	export constexpr inline uint_t ID_MAX_COUNT = (1ull << ID_BITS);
	export constexpr inline uint_t ID_MAX = ID_MAX_COUNT - 1;
	export const uint_t			   ID_NULL = ID_MAX;
	export const uint_t			   ARCHETYPE_COUNT = (1ull << ARCHETYPE_BITS);

	export constexpr inline uint_t SPARSE_INDEX_NULL = std::numeric_limits<uint_t>::max();

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

		inline uint_t get_id() const noexcept { return id; }
		inline uint_t get_version() const noexcept { return ver; }
		inline uint_t get_flag() const noexcept { return flag; }
		inline uint_t get_arche() const noexcept { return arche; }

		inline void set_id(uint_t id_) noexcept { id = id_; }
		inline void inc_id() noexcept { ++id; }
		inline void set_version(uint_t ver_) noexcept { ver = ver_; }
		inline void inc_version() noexcept { ++ver; }
		inline void set_flag(uint_t flag_) noexcept { flag = flag_; }
		inline void set_arche(uint_t arche_) noexcept { arche = arche_; }

		inline bool is_same(entity_t other)noexcept
		{
			if (other.id != id) { return false; }
			if (other.ver != ver) { return false; }
			if (other.arche != arche) { return false; }
			return true;
		}

		inline bool is_null() const noexcept  { return id == ID_NULL; }
		inline void recycle() noexcept { inc_version(); flag = 0; }

		template<uint_t ChunkSize>
		inline uint_t chunk_id() const noexcept { return id / ChunkSize; }
		template<uint_t ChunkSize>
		inline uint_t sparse_id() const noexcept { return id & (ChunkSize - 1); }
	};
	entity_t entity_t::null = { ID_NULL, 0, 0, 0 };


	export template<size_t ArcheId>
	class id_generator_t
	{
	private:
		entity_t current;
		std::queue<entity_t> free;

	public:
		id_generator_t() noexcept { current = entity_t{ 0,0,0,ArcheId }; }

		entity_t get() noexcept
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

		void recycle(entity_t e) noexcept
		{
			e.recycle();
			free.push(e);
		}

		bool full() noexcept { return current.get_id() == ID_MAX;}

		uint_t available() noexcept {return free.size() + ID_MAX - current.get_id(); }
	};

	export template<size_t FromArcheId, size_t GlobalArcheId>
	struct sparse_index_global_t
	{
	private:
		static xvector<uint_t> indexies;
	public:
		static uint_t try_dense_id(uint_t sparse_id, bool larger_than_resize)
		{
			if (sparse_id < indexies.size())[[likely]]
			{
				return indexies[sparse_id];
			}
			else if(larger_than_resize)
			{
				spinlock_t spin;
				spin.lock();
				if (sparse_id >= indexies.size())
				{
					auto newsize = (uint_t)(sparse_id * 1.4);
					newsize = newsize >= ID_MAX_COUNT ? ID_MAX_COUNT : newsize;
					indexies.resize(newsize, SPARSE_INDEX_NULL);
				}
				spin.unlock();
			}
			return SPARSE_INDEX_NULL;
		}
		static void set_dense_id(uint_t sparse_id, uint_t dense_id)
		{ 
			indexies[sparse_id] = dense_id;
		}
	};
	template<size_t FromArcheId, size_t GlobalArcheId> 
	xvector<uint_t> sparse_index_global_t<FromArcheId, GlobalArcheId>::indexies(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT, SPARSE_INDEX_NULL);

	enum entity_state
	{
		miss,
		outdate,
		exist,
	};

	enum archetype_state_e
	{
		idle,
		read,
		write,
	};

	export enum archelayout_e
	{
		paged,
		global,
		singleton,
	};

	template<archelayout_e Layout, size_t ChunkMemSize = configs::ARCHETYPE_STANDARD_CHUNK_MEM_SIZE, size_t ChunkMinCapcity = 1, size_t ChunkMaxCapcity = 512>
	struct archepolicy_t
	{
		const archelayout_e LAYOUT = Layout;
		const size_t CHUNK_MEM_SIZE = ChunkMemSize;
		const size_t CHUNK_MIN_CAPACITY = ChunkMinCapcity;
		const size_t CHUNK_MAX_CAPACITY = ChunkMaxCapcity;
	};

	using archepolicy_paged_defalut_t = archepolicy_t<paged>;
	using archepolicy_singleton_t = archepolicy_t<singleton, 1, 1, 1>;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	export template<typename... Args> struct sparse_set_global_t;
	export template<typename... Args>
	struct sparse_set_global_t<mpl::typelist<Args...>>
	{
		static constexpr size_t ROW_SIZE = (misc::data_size_of_t<Args...>::value + sizeof(entity_t));
		static constexpr size_t decide_element_per_chunk()
		{
			size_t count = maths::round_power_2(configs::ARCHETYPE_GLOBAL_CHUNK_MEM_SIZE / ROW_SIZE);
			count = count < 1 ? 1 : count;
			return count;
		}
		static constexpr size_t COUNT_PER_CHUNK = decide_element_per_chunk();

		static inline uint_t to_chunk_paged_id(uint_t dense_id) 
		{ return dense_id & (COUNT_PER_CHUNK - 1); }
	

		////////////////////////////////////////////////////////////////////////////////////////////////
		// 
		////////////////////////////////////////////////////////////////////////////////////////////////
		template<typename T>
		struct column_t
		{
			using elements_t = xarray<T, COUNT_PER_CHUNK>;
			elements_t values{};

			T& operator[](uint_t index) noexcept { return values[index]; }
			const T& operator[](uint_t index) const noexcept { return values[index]; }

			elements_t::iterator begin() noexcept { return values.begin(); }

			elements_t::const_iterator end() const noexcept { return values.end(); }
		};
		template<>
		struct column_t<entity_t>
		{
			using elements_t = xarray<entity_t, COUNT_PER_CHUNK>;
			elements_t values = utility::make_array<entity_t, COUNT_PER_CHUNK>(entity_t::null);

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
			uint_t length = 0;

		public:
			uint_t get_length() { return length; }
			void incr_length() { length += 1; }
			void decr_iength() { length -= 1; }

		private:
			template<typename T> auto& column() noexcept { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

			template<typename T> const auto& column() const noexcept { return column_t<mpl::traits::remove_qualifier_t<T>>::values; }

		public:
			template<typename T>
			inline T& get_component(uint_t dense_index) noexcept
			{
				auto& container = column<T>();
				return container[dense_index];
			}
			template<typename T>
			inline const T& get_component(uint_t dense_index) const noexcept 
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

			template<typename... Ts>
			inline bool create_components(uint_t dense_index, entity_t ent, Ts&&...args)
			{
				entity_t& target_ent = get_component<entity_t>(dense_index);

				((set_component<Ts>(dense_index, std::forward<Ts>(args))), ...);
				set_component<entity_t>(dense_index, std::move(ent));

				length += 1;
				return true;
			}

			template<typename... Ts>
			inline bool set_components(uint_t dense_index, entity_t ent, Ts&&...args)
			{
				//static_assert(!mpl::has_raw_of_v<mpl::typelist<Ts...>, entity_t>, "entity_t to be key only, not allow to be part of updates");
				entity_t& target_ent = get_component<entity_t>(dense_index);
				//must be the same entity
				if (!target_ent.is_same(ent))
				{
					return false;
				}
				((set_component<Ts>(dense_index, std::forward<Ts>(args))), ...);

				return true;
			}

			template<typename Lambda, typename... Ts>
			inline void each(Lambda&& lambda) noexcept
			{
				for (uint_t i = 0; i < length; i++)
				{
					lambda(std::forward<Ts>(column<Ts>()[i])...);
				}
			}
		};


		////////////////////////////////////////////////////////////////////////////////////////////////
		// begin sparse_set_paged_t main declare
		////////////////////////////////////////////////////////////////////////////////////////////////

		using chunk_ptrs_t = xvector<chunk_t*>;
		using try_dense_id_ptr = uint_t(*)(entity_t, bool);
		using set_dense_id_ptr = void(*)(entity_t, uint_t);

	private:
		chunk_ptrs_t chunks{};
		uint_t chunks_count = 0;
		uint_t elements_count = 0;
	public:
		try_dense_id_ptr try_dense_id;
		set_dense_id_ptr set_dense_id;

	public:
		inline chunk_t* try_get_chunk(uint_t ichunk, bool larger_then_resize)
		{
			if(ichunk < chunks_count)
			{ 
				return chunks[ichunk]; 
			}
			else if(larger_then_resize)
			{
				chunk_t* chunk_ptr = nullptr;
				spinlock_t spin;
				spin.lock();
				if (ichunk >= chunks_count)
				{
					chunk_ptr = new chunk_t{};
					chunks.push_back(chunk_ptr);
					chunks_count++;
				}
				spin.unlock();
				return chunk_ptr;
			}

			return nullptr;
		}

		inline entity_state has_entity(entity_t ent)
		{
			uint_t dense_id = try_dense_id(ent, false);
			if(SPARSE_INDEX_NULL == dense_id)
			{ 
				return entity_state::miss; 
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			entity_t latest = chunk_ptr->get_component<entity_t>(to_chunk_paged_id(dense_id));
			if(!latest.is_same(ent))
			{ return entity_state::outdate; }

			return entity_state::exist;
		}

		template<typename... Ts>
		inline bool create_components(entity_t ent, Ts&&...args)
		{
			uint_t dense_id = try_dense_id(ent, true);
			if (SPARSE_INDEX_NULL == dense_id)
			{
				dense_id = elements_count;
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, true);
			set_dense_id(ent, dense_id);
			dense_id = to_chunk_paged_id(dense_id);
			bool ok = chunk_ptr->create_components(dense_id, ent, std::forward<Ts>(args)...);
			elements_count += 1;
			return ok;
		}

		template<typename... Ts>
		inline bool set_components(entity_t ent, Ts&&...args)
		{
			//static_assert(!mpl::has_raw_of_v<mpl::typelist<Ts...>, entity_t>, "entity_t to be key only, not allow to be part of updates");
			if(ent.is_null())
			{ return false; }

			uint_t dense_id = try_dense_id(ent, false);
			if(SPARSE_INDEX_NULL == dense_id)
			{ return false; }
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			if(nullptr == chunk_ptr)
			{ return false; }

			dense_id = to_chunk_paged_id(dense_id);
			bool ok = chunk_ptr->set_components(dense_id, ent, std::forward<Ts>(args)...);
			return ok;
		}

		bool remove(entity_t ent) 
		{
			if(ent.is_null())
			{ return false; }

			uint_t dense_id = try_dense_id(ent, false);
			if(SPARSE_INDEX_NULL == dense_id)
			{ return false;
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			if (nullptr == chunk_ptr)
			{ return false; }

			if(elements_count == 0)
			{ return false; }

			uint_t last_dense_id = elements_count - 1;
			uint_t last_ichunk = last_dense_id / COUNT_PER_CHUNK;
			chunk_t* last_chunk_ptr = try_get_chunk(last_ichunk, false);

			if(dense_id == last_dense_id)
			{
				set_dense_id(ent, SPARSE_INDEX_NULL);
				elements_count -= 1;
				last_chunk_ptr->decr_iength();
				return true;
			}


			const entity_t& last_ent = last_chunk_ptr->get_component<entity_t>(to_chunk_paged_id(last_dense_id));
			(chunk_ptr->set_components<Args>(to_chunk_paged_id(dense_id), 
											 ent,
				                             std::forward<Args>(last_chunk_ptr->get_component<Args>(to_chunk_paged_id(last_dense_id)))), ...);

			set_dense_id(last_ent, dense_id);
			set_dense_id(ent, SPARSE_INDEX_NULL);
			if (to_chunk_paged_id(elements_count) == 0)
			{ chunks_count -= 1; }
			elements_count -= (elements_count < 0 ? 0 : 1);
			last_chunk_ptr->decr_iength();
			return true;
		}

		template<typename Lambda, typename... Ts>
		inline void each(Lambda&& lambda) noexcept
		{
			for (uint_t i = 0; i < chunks_count; i++)
			{
				chunk_t* chunk_ptr = chunks[i];
				chunk_ptr->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
			}
		}

		//chunk_ptrs_t::const_iterator begin() noexcept { return chunks.begin(); }

		//chunk_ptrs_t::const_iterator end() const noexcept { return chunks.begin() + chunks_count; }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args>
	struct sort_component_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_unpack_t<entity_t, Args...>>;

		template<size_t... I>
		static constexpr auto apply_impl(std::index_sequence<I...>)
		{
			using type = mpl::typelist<mpl::at_t<I, unpack_list_t>...>;
			return type{};
		}

		static constexpr auto apply()
		{
			constexpr auto storage = apply_impl(
				misc::constexpr_array_to_index_sequence < 
					misc::sort_index_sequence(unpack_list_t{}) > (
						std::make_index_sequence<mpl::length<unpack_list_t>::value>{}));

			return storage;
		}

	public:
		using type = decltype(apply());
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename Derived> struct archetype_base_t;
	template<typename... Args> struct archetype_global_t;

	export template <class C>
	concept IsDerivedArchetypeConcept = requires(C c)
	{
		// IILE
		[] <typename... X>(archetype_base_t<X...>&) {}(c);
	};

	export template<typename T> struct is_global_archetype_t { static constexpr inline bool value = 0; };
	export template<typename... TList> struct is_global_archetype_t<archetype_global_t<TList...>> { static constexpr inline bool value = 1; };
	export template<typename T> constexpr bool is_global_archetype_v = is_global_archetype_t<T>::value;

	template<typename T, typename=void> struct archetype_global_list_helper { using type = mpl::nulllist; };
	template<typename T>
	struct archetype_global_list_helper<T, std::void_t<decltype(sizeof(T::global_list))>>
	{ 
		using type = T::global_list; 
	};

	template<typename T, typename = void> struct archetype_atom_list_helper { using type = mpl::nulllist; };
	template<typename T>
	struct archetype_atom_list_helper<T, std::void_t<decltype(sizeof(T::local_list))>>
	{
		using type = T::local_list;
	};

	template<typename... TList> struct archetype_global_atom_type;
	template<typename T, typename... TList> 
	struct archetype_global_atom_type<mpl::typelist<T, TList...>>
	{
		using global_list = mpl::traits::if_else_t< is_global_archetype_v<T>,
													mpl::concat_t<T, typename archetype_global_atom_type<mpl::typelist<TList...>>::global_list>,
													mpl::traits::if_else_t<
														IsDerivedArchetypeConcept<T>,
														mpl::concat_t<typename archetype_global_list_helper<T>::type, typename archetype_global_atom_type<mpl::typelist<TList...>>::global_list>,
														typename archetype_global_atom_type<mpl::typelist<TList...>>::global_list
																			>
												    >;


		using local_list = mpl::traits::if_else_t<!IsDerivedArchetypeConcept<T>,
												  mpl::concat_t<T, typename archetype_global_atom_type<mpl::typelist<TList...>>::local_list>,
												  mpl::concat_t<typename archetype_atom_list_helper<T>::type, typename archetype_global_atom_type<mpl::typelist<TList...>>::local_list>
											    >;
	};
	template<>
	struct archetype_global_atom_type<mpl::nulllist>
	{
		using global_list = mpl::nulllist;
		using local_list = mpl::nulllist;
	};
	
	export template<typename Derived>
	struct archetype_base_t
	{
		using raw_list = mpl::to_list_t<Derived>;
		using global_list = mpl::traits::if_else_t<is_global_archetype_v<Derived>, mpl::typelist<Derived>, typename archetype_global_atom_type<raw_list>::global_list>;
		using local_list = mpl::traits::if_else_t < is_global_archetype_v<Derived>, mpl::nulllist, typename archetype_global_atom_type<raw_list>::local_list>;
		using refine_list = mpl::traits::if_else_t<is_global_archetype_v<Derived>, raw_list, local_list>;
		using order_list = sort_component_t<mpl::unpack_unpack_t<local_list, global_list>>::type;

		static inline constexpr size_t this_id = misc::id_by_archetype<Derived>;
		enum { map_component_archetype = misc::map_component_archetype<this_id, order_list>::value };

		static const char* name() { return misc::type_name_id<Derived>(); }

		static bool allocated() noexcept { return Derived::allocated_impl(); }

		static uint_t available() noexcept { return Derived::available_impl(); }

		static bool has_entity(entity_t ent) { return Derived::has_entity_impl(ent); }

		template<typename... Ts>
		static entity_t create(Ts&&...args)
		{
			return Derived::create_impl(std::forward<Ts>(args)...);
		}

		static bool remove(entity_t ent)
		{
			return Derived::remove_impl(ent);
		}

		template<typename... Ts>
		static void each(void(*func)(Ts...)) noexcept
		{
			Derived::each_impl(func, mpl::typelist<Ts...>{});
		}

		template<typename Lambda>
		static void each(Lambda&& lambda) noexcept
		{
			using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
			Derived::each_impl(std::forward<Lambda>(lambda), args_type{});
		}
	};

	//template<typename... Args> struct archetype_global_t;
	export template<typename... Args>
	struct archetype_global_t : archetype_base_t<archetype_global_t<Args...>>
	{
	private:
		using base_type = archetype_base_t<archetype_global_t<Args...>>;
		using storage_t = sparse_set_global_t<typename base_type::order_list>;
		using generator_id_t = id_generator_t<base_type::this_id>;

		////////////////////////////////////////////////////////////////////////////////////////////////
		// 
		////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		using try_dense_id_ptr = uint_t(*)(uint_t, bool);
		using set_dense_id_ptr = void(*)(uint_t, uint_t);
		using try_id_func_array_t = xarray< try_dense_id_ptr, ARCHETYPE_COUNT>;
		using set_id_func_array_t = xarray< set_dense_id_ptr, ARCHETYPE_COUNT>;
		static inline try_id_func_array_t try_dense_id_func_array;
		static inline set_id_func_array_t set_dense_id_func_array;
	public:

		template<size_t ...Is>
		static constexpr auto make_sparse_indexer_ptr(mpl::seq::indexlist<Is...>)
		{
			constexpr xarray<uint_t, sizeof...(Is)> archeids = { Is... };

			constexpr xarray< try_dense_id_ptr, sizeof...(Is)> try_dense_id_ptrs = { (&sparse_index_global_t<Is, base_type::this_id>::try_dense_id)... };
			for (uint_t i = 0; i < archeids.size(); i++)
			{
				try_dense_id_func_array[archeids[i]] = try_dense_id_ptrs[i];
			}

			constexpr xarray<set_dense_id_ptr, sizeof...(Is)> set_dense_id_ptrs = { (&sparse_index_global_t<Is, base_type::this_id>::set_dense_id)... };
			for (uint_t i = 0; i < archeids.size(); i++)
			{
				set_dense_id_func_array[archeids[i]] = set_dense_id_ptrs[i];
			}
		}
	private:
		static inline uint_t try_dense_id(entity_t ent, bool larger_then_resize)
		{
			uint_t dense_id = try_dense_id_func_array[ent.get_arche()](ent.get_id(), larger_then_resize);
			return dense_id;
		}
		static inline void set_dense_id(entity_t ent, uint_t dense_id)
		{
			set_dense_id_func_array[ent.get_arche()](ent.get_id(), dense_id);
		}
	private:
		static generator_id_t id_generator;
		static storage_t* sparse_set_ptr;

		static storage_t* try_sparse_set()
		{
			if (nullptr == sparse_set_ptr)
			{
				spinlock_t spin;
				spin.lock();
				if (nullptr == sparse_set_ptr)
				{
					sparse_set_ptr = new storage_t();
					sparse_set_ptr->try_dense_id = &try_dense_id;
					sparse_set_ptr->set_dense_id = &set_dense_id;
				}
				spin.unlock();
			}
			return sparse_set_ptr;
		}

	public:
		static bool allocated_impl() noexcept { return sparse_set_ptr != nullptr; }

		static uint_t available_impl() noexcept { return id_generator.available(); }

		static bool has_entity_impl(entity_t ent) noexcept 
		{
			if (!allocated_impl()) [[unlikely]]
			{ return false; }

			entity_state state_ = sparse_set_ptr->has_entity(ent);
			return state_ == entity_state::exist;
		}

		template<typename... Ts>
		static entity_t create_impl(Ts&&...args)
		{
			entity_t ent = id_generator.get();
			storage_t* sparse_ptr = try_sparse_set();
			sparse_ptr->create_components(ent, std::forward<Ts>(args)...);

			return ent;
		}

		static bool remove_impl(entity_t ent)
		{
			if (! allocated_impl()) [[unlikely]]
			{ return false; }
			bool ok = sparse_set_ptr->remove(ent);
			return ok;
		}

		template<typename Lambda, typename... Ts>
		static void each_impl(Lambda&& lambda, mpl::typelist<Ts...>) noexcept
		{
			static_assert(!mpl::has_raw_reference_of_v<mpl::typelist<Ts...>, entity_t>, "each() function: forbidden reference to entity_t");
			if (! allocated_impl()) [[unlikely]]
			{ return; }

			sparse_set_ptr->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
		}
	};
	template<typename... Args> archetype_global_t<Args...>::storage_t*  archetype_global_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> archetype_global_t<Args...>::generator_id_t  archetype_global_t<Args...>::id_generator{};

	////////////////////////////////////////////////////////////////////////////////////////////////
	export  template<typename... Args>
	struct archetype_standard_t : archetype_base_t<archetype_standard_t<Args...>>
	{
		using this_type = archetype_standard_t<Args...>;
		using base_type = archetype_base_t<archetype_standard_t<Args...>>;
		using storage_t = sparse_set_global_t<typename base_type::order_list>;
		using generator_id_t = id_generator_t<base_type::this_id>;

	private:
		static inline uint_t try_dense_id(entity_t ent, bool larger_then_resize)
		{
			uint_t dense_id = sparse_index_global_t<base_type::this_id, base_type::this_id>::try_dense_id(ent.get_id(), larger_then_resize);
			return dense_id;
		}
		static inline void set_dense_id(entity_t ent, uint_t dense_id)
		{
			sparse_index_global_t<base_type::this_id, base_type::this_id>::set_dense_id(ent.get_id(), dense_id);
		}
	private:
		static storage_t* sparse_set_ptr;
		static generator_id_t id_generator;

		static storage_t* try_sparse_set()
		{
			if (nullptr == sparse_set_ptr)
			{
				spinlock_t spin;
				spin.lock();
				if (nullptr == sparse_set_ptr)
				{
					sparse_set_ptr = new storage_t();
					sparse_set_ptr->try_dense_id = &try_dense_id;
					sparse_set_ptr->set_dense_id = &set_dense_id;
				}
				spin.unlock();
			}
			return sparse_set_ptr;
		}


	public:
		static bool allocated_impl() { return sparse_set_ptr != nullptr; }

		static uint_t available_impl() { return id_generator.available(); }

		template<typename... Ts>
		static entity_t create_impl(Ts&&...args) 
		{
			entity_t ent = id_generator.get();
			storage_t* sparse_ptr = try_sparse_set();
			sparse_ptr->create_components(ent, std::forward<Ts>(args)...);

			return ent;
		}

		static void remove_impl(entity_t ent)
		{
			if(nullptr == sparse_set_ptr) [[unlikely]]
			{ return; }
			bool ok = sparse_set_ptr->remove(ent);
			return ok;
		}

		template<typename Lambda, typename... Ts>
		static void each_impl(Lambda&& lambda, mpl::typelist<Ts...>)
		{
			static_assert(!mpl::has_raw_reference_of_v<mpl::typelist<Ts...>, entity_t>, "each() function: forbidden reference to entity_t");
			if (nullptr == sparse_set_ptr) [[unlikely]]
			{
				return;
			}
			sparse_set_ptr->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
		}
	};
	template<typename... Args> typename archetype_standard_t<Args...>::storage_t* archetype_standard_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> archetype_standard_t<Args...>::generator_id_t archetype_standard_t<Args...>::id_generator{};

	////////////////////////////////////////////////////////////////////////////////////////////////

	export template <class C>
	concept HasEachMethod = std::is_function_v<decltype(C::each)>;

	export template<class C> concept IsTemplateClassConcept = requires(C c)
	{
		[] <template<class...> class T, class... Ts>(T<Ts...>) {}(c);
	};

	//export template<typename... Arche>
	export struct behaviour_base_t {};
	export template <class C>
	concept IsDerivedBehaviourConcept = std::is_base_of_v<behaviour_base_t, C>;

	//export template <class C>
	//concept IsDerivedBehaviourConcept = requires(C c)
	//{
	//	// IILE
	//	[] <typename... X>(behaviour_base_t<X...>&) {}(c);
	//};
}




