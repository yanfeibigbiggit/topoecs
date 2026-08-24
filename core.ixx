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

	template<size_t GlobalArcheId>
	struct sparse_index_global_t
	{
	private:
		using row_t = xvector<uint_t>;
		using rows_t = xvector<row_t*>;
		static rows_t indexies;
	public:
		static uint_t try_dense_id(uint_t arche_id, uint_t sparse_id, bool larger_than_resize)
		{
			if (arche_id < indexies.size()) [[likely]]
			{
			SPARSE_INDEX_GLOBAL_RESIZE_LABEL:
				auto row_ptr = indexies[arche_id];
				if (nullptr == row_ptr) [[unlikely]]
				{
					if (!larger_than_resize)
					{
						return SPARSE_INDEX_NULL;
					}
					spinlock_t spin;
					spin.lock();
					row_ptr = indexies[arche_id];
					if (nullptr == row_ptr)
					{
						row_ptr = new row_t(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT, SPARSE_INDEX_NULL);
						indexies[arche_id] = row_ptr;
					}
					spin.unlock();
				}
				if (sparse_id < row_ptr->size()) [[likely]]
				{
					return row_ptr->at(sparse_id);
				}
				else if(larger_than_resize)
				{
					spinlock_t spin;
					spin.lock();
					if (sparse_id >= row_ptr->size())
					{
						auto newsize = sparse_id > row_ptr->capacity() ? sparse_id : row_ptr->capacity();
						newsize = (uint_t)(newsize * 1.4);
						newsize = newsize >= ID_MAX ? ID_MAX : newsize;
						row_ptr->resize(newsize, SPARSE_INDEX_NULL);
					}
					spin.unlock();
				}
				return SPARSE_INDEX_NULL;
			}
			else if(larger_than_resize)
			{
				spinlock_t spin;
				spin.lock();
				if (arche_id >= indexies.size())
				{
					auto newsize = arche_id > indexies.capacity() ? arche_id : indexies.capacity();
					newsize = (uint_t)(newsize * 1.4);
					newsize = newsize >= ARCHETYPE_COUNT ? ARCHETYPE_COUNT : newsize;
					indexies.resize(newsize, nullptr);
				}
				spin.unlock();
				goto SPARSE_INDEX_GLOBAL_RESIZE_LABEL;
			}
			return SPARSE_INDEX_NULL;
		}
		static void set_dense_id(uint_t arche_id, uint_t sparse_id, uint_t dense_id)
		{ 
			if (arche_id < indexies.size())
			{
				auto row_ptr = indexies[arche_id];
				if (nullptr != row_ptr && sparse_id < row_ptr->size())
				{
					(*row_ptr)[sparse_id] = dense_id;
				}
			}
		}
	};
	template<size_t GlobalArcheId> 
	xvector<xvector<uint_t>*> sparse_index_global_t<GlobalArcheId>::indexies(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT, nullptr);

	template<size_t FromArcheId>
	struct sparse_index_standard_t
	{
	private:
		static xvector<uint_t> indexies;
	public:
		static uint_t try_dense_id(uint_t sparse_id, bool larger_than_resize)
		{
			if (sparse_id < indexies.size()) [[likely]]
			{
				return indexies[sparse_id];
			}
			else if (larger_than_resize)
			{
				spinlock_t spin;
				spin.lock();
				if (sparse_id >= indexies.size())
				{
					auto newsize = (uint_t)(sparse_id * 1.4);
					newsize = newsize >= ID_MAX ? ID_MAX : newsize;
					indexies.resize(newsize, SPARSE_INDEX_NULL);
				}
				spin.unlock();
			}
			return SPARSE_INDEX_NULL;
		}
		static void set_dense_id(uint_t sparse_id, uint_t dense_id)
		{
			if(sparse_id < indexies.size())
				indexies[sparse_id] = dense_id;
		}
	};
	template<size_t FromArcheId>
	xvector<uint_t> sparse_index_standard_t<FromArcheId>::indexies(configs::ARCHETYPE_SPARSESET_SPARSE_ARRAY_INIT, SPARSE_INDEX_NULL);

	enum entity_state
	{
		miss,
		outdate,
		alive,
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

	//template<typename... Args> struct to_archetype_global_type;
	//template<typename... Args>
	//struct to_archetype_global_type<mpl::typelist<Args...>>
	//{
	//	using type = archetype_global_t<Args...>;
	//};
	//template<typename Args> using to_archetype_global_t = to_archetype_global_type<Args>::type;
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename Arche, typename... Args> struct sparse_set_paged_t;
	template<typename Arche, typename... Args>
	struct sparse_set_paged_t<Arche, mpl::typelist<Args...>>
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

			//T& operator[](uint_t index) noexcept { return values[index]; }
			//const T& operator[](uint_t index) const noexcept { return values[index]; }
		};
		template<>
		struct column_t<entity_t>
		{
			using elements_t = xarray<entity_t, COUNT_PER_CHUNK>;
			elements_t values = utility::make_array<entity_t, COUNT_PER_CHUNK>(entity_t::null);

			//entity_t& operator[](uint_t index) noexcept { return values[index]; }
			//const entity_t& operator[](uint_t index) const noexcept { return values[index]; }
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
			inline T& get(uint_t dense_index, entity_t ent) noexcept
			{
				using global_args = mpl::firstlist_has_t<mpl::to_list_list_t<Arche::global_list>, mpl::traits::remove_qualifier_t<T>>;
				if constexpr (std::is_same_v<global_args, mpl::nulllist>)
				{
					return column<T>()[dense_index];
				}
				else
				{
					//using tglobal = to_archetype_global_t<global_args>;
					constexpr size_t global_index = mpl::indexof_v<global_args, mpl::to_list_list_t<Arche::global_list>>;
					using tglobal = mpl::at_t<global_index, Arche::global_list>;
					if constexpr(std::is_const_v<T>)
					{
						return tglobal::template get<T>(ent);
					}
					else
					{
						return tglobal::template get_ref<T>(ent);
					}
				}
			}
			template<typename T>
			inline const T& get(uint_t dense_index, entity_t ent) const noexcept
			{
				using global_args = mpl::firstlist_has_t<mpl::to_list_list_t<Arche::global_list>, mpl::traits::remove_qualifier_t<T>>;
				if constexpr (std::is_same_v<global_args, mpl::nulllist>)
				{
					return column<T>()[dense_index];
				}
				else
				{
					//using tglobal = to_archetype_global_t<global_args>;
					constexpr size_t global_index = mpl::indexof_v<global_args, mpl::to_list_list_t<Arche::global_list>>;
					using tglobal = mpl::at_t<global_index, Arche::global_list>;
					if constexpr (std::is_const_v<T>)
					{
						return tglobal::template get<T>(ent);
					}
					else
					{
						return tglobal::template get_ref<T>(ent);
					}
				}
			}

			template<typename T>
			inline T& get(uint_t dense_index) noexcept
			{
				auto& container = column<T>();
				return container[dense_index];
			}
			template<typename T>
			inline const T& get(uint_t dense_index) const noexcept 
			{
				const auto& container = column<T>();
				return container[dense_index];
			}

			template<typename T>
			inline void set(uint_t dense_index, T&& arg)
			{
				auto& container = column<T>();
				container[dense_index] = std::forward<T>(arg);
			}

			template<typename... Ts>
			inline bool creates(uint_t dense_index, entity_t ent, Ts&&...args)
			{
				((set<Ts>(dense_index, std::forward<Ts>(args))), ...);
				set<entity_t>(dense_index, std::move(ent));

				length += 1;
				return true;
			}

			template<typename... Ts>
			inline bool sets(uint_t dense_index, entity_t ent, Ts&&...args)
			{
				//static_assert(!mpl::has_raw_of_v<mpl::typelist<Ts...>, entity_t>, "entity_t to be key only, not allow to be part of updates");
				entity_t& target_ent = get<entity_t>(dense_index);
				//must be the same entity
				if (!ent.is_same(target_ent))
				{
					return false;
				}
				((set<Ts>(dense_index, std::forward<Ts>(args))), ...);

				return true;
			}

			template<typename Lambda, typename... Ts>
			inline void each(Lambda&& lambda) noexcept
			{
				using component_globals = mpl::listlist_has_any_of_t<mpl::to_list_list_t<typename Arche::global_list>, mpl::list_remove_qualifier_t<mpl::typelist<Ts...>>>;	
				if constexpr(mpl::length_v<component_globals> == 0)
				{
					for (uint_t i = 0; i < length; i++)
					{
						lambda(std::forward<Ts>(get<Ts>(i))...);
					}
				}
				else
				{
					for (uint_t i = 0; i < length; i++)
					{
						entity_t ent = get<entity_t>(i);
						lambda(std::forward<Ts>(get<Ts>(i, ent))...);
					}
				}
			}
		};


		////////////////////////////////////////////////////////////////////////////////////////////////
		// begin sparse_set_paged_t main declare
		////////////////////////////////////////////////////////////////////////////////////////////////

		using chunk_ptrs_t = xvector<chunk_t*>;

	private:
		chunk_ptrs_t chunks{};
		uint_t chunks_count = 0;
		uint_t elements_count = 0;

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
			uint_t dense_id = Arche::try_dense_id(ent, false);
			if(SPARSE_INDEX_NULL == dense_id)
			{ 
				return entity_state::miss; 
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			entity_t latest = chunk_ptr->get<entity_t>(to_chunk_paged_id(dense_id));
			if(!latest.is_same(ent))
			{ return entity_state::outdate; }

			return entity_state::alive;
		}

		template<typename... Ts>
		inline bool creates(entity_t ent, Ts&&...args)
		{
			uint_t dense_id = Arche::try_dense_id(ent, true);
			if (SPARSE_INDEX_NULL != dense_id)
			{ return false; }
			dense_id = elements_count;
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, true);
			Arche::set_dense_id(ent, dense_id);
			dense_id = to_chunk_paged_id(dense_id);
			bool ok = chunk_ptr->creates(dense_id, ent, std::forward<Ts>(args)...);
			elements_count += 1;
			return ok;
		}

		template<typename... Ts>
		inline bool sets(entity_t ent, Ts&&...args)
		{
			//static_assert(!mpl::has_raw_of_v<mpl::typelist<Ts...>, entity_t>, "entity_t to be key only, not allow to be part of updates");
			//if(ent.is_null())
			//{ return false; }

			uint_t dense_id = Arche::try_dense_id(ent, false);
			if(SPARSE_INDEX_NULL == dense_id)
			{ return false; }
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			if(nullptr == chunk_ptr)
			{ return false; }

			dense_id = to_chunk_paged_id(dense_id);
			bool ok = chunk_ptr->sets(dense_id, ent, std::forward<Ts>(args)...);
			return ok;
		}

		template<typename... Ts>
		inline bool create_or_set(entity_t ent, Ts&&...args)
		{
			uint_t dense_id = Arche::try_dense_id(ent, true);
			if (SPARSE_INDEX_NULL == dense_id)
			{
				return creates(ent, std::forward<Ts>(args)...);
			}
			else
			{
				return sets(ent, std::forward<Ts>(args)...);
			}
		}

		template<typename T>
		inline T& get(entity_t ent)
		{
			uint_t dense_id = Arche::try_dense_id(ent, false);
			if (SPARSE_INDEX_NULL == dense_id) [[unlikely]]
			{
				throw std::exception(std::format("component {} miss at entity: id:{} ver:{} arche:{}", typeid(T).name(), ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);
			dense_id = to_chunk_paged_id(dense_id);
			return chunk_ptr->get<T>(dense_id);
		}

		template<typename T>
		inline const T& get(entity_t ent) const
		{
			uint_t dense_id = Arche::try_dense_id(ent, false);
			if (SPARSE_INDEX_NULL == dense_id) [[unlikely]]
			{
				throw std::exception(std::format("component {} miss at entity: id:{} ver:{} arche:{}", typeid(T).name(), ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
			}
			uint_t ichunk = dense_id / COUNT_PER_CHUNK;
			chunk_t* chunk_ptr = try_get_chunk(ichunk, false);

			dense_id = to_chunk_paged_id(dense_id);
			return chunk_ptr->get<T>(dense_id);
		}

		bool remove(entity_t ent) 
		{
			if(ent.is_null())
			{ return false; }

			uint_t dense_id = Arche::try_dense_id(ent, false);
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
				Arche::set_dense_id(ent, SPARSE_INDEX_NULL);
				elements_count -= 1;
				last_chunk_ptr->decr_iength();
				return true;
			}

			const entity_t& last_ent = last_chunk_ptr->get<entity_t>(to_chunk_paged_id(last_dense_id));
			(chunk_ptr->sets<Args>(to_chunk_paged_id(dense_id), 
											 ent,
				                             std::forward<Args>(last_chunk_ptr->get<Args>(to_chunk_paged_id(last_dense_id)))), ...);

			Arche::set_dense_id(last_ent, dense_id);
			Arche::set_dense_id(ent, SPARSE_INDEX_NULL);
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
		using order_list = sort_component_t<mpl::unpack_unpack_t<local_list, global_list>>::type;
		using refine_list = mpl::no_duplicate_t<mpl::unpack_unpack_t<mpl::push_front_t<entity_t, mpl::traits::if_else_t<is_global_archetype_v<Derived>, raw_list, local_list>>>>;
		static inline constexpr size_t this_id = misc::id_by_archetype<Derived>;
	private:
		enum { map_component_archetype = misc::map_component_archetype<this_id, order_list>::value };
	public:
		static const char* name() { return misc::type_name_id<Derived>(); }

		static bool allocated() noexcept { return Derived::allocated_impl(); }

		static uint_t available() noexcept { return Derived::available_impl(); }

		static bool has_entity(entity_t ent) { return Derived::has_entity_impl(ent); }

		template<typename... Ts>
		static entity_t create(Ts&&...args)
		{
			static_assert(mpl::is_sub_v<order_list, mpl::list_remove_qualifier_t<mpl::typelist<Ts...>>>, "args of create are not belong to archetype");
			return Derived::create_impl(std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		static entity_t set(entity_t ent, Ts&&... args)
		{
			return Derived::set_impl(ent, std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		static bool create_or_set(entity_t ent, Ts&&... args)
		{
			return Derived::create_or_set_impl(ent, std::forward<Ts>(args)...);
		}

		static bool remove(entity_t ent)
		{
			return Derived::remove_impl(ent);
		}

		template<typename T>
		static T& get_ref(entity_t ent)
		{
			return Derived::template get_ref_impl<T>(ent);
		}

		template<typename T>
		static const T& get(entity_t ent)
		{
			return Derived::template get_impl<T>(ent);
		}

		template<typename... Ts>
		static void each(void(*func)(Ts...)) noexcept
		{
			static_assert(mpl::is_sub_v<order_list, mpl::list_remove_qualifier_t<mpl::typelist<Ts...>>>, "args of lambda are not belong to archetype");
			Derived::each_impl(func, mpl::typelist<Ts...>{});
		}

		template<typename Lambda>
		static void each(Lambda&& lambda) noexcept
		{
			using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
			static_assert(mpl::is_sub_v<order_list, mpl::list_remove_qualifier_t<args_type>>, "args of lambda are not belong to archetype");
			Derived::each_impl(std::forward<Lambda>(lambda), args_type{});
		}
	};

	//template<typename... Args> struct archetype_global_t;
	export template<typename... Args>
	struct archetype_global_t final : archetype_base_t<archetype_global_t<Args...>>
	{
	private:
		using base_type = archetype_base_t<archetype_global_t<Args...>>;
		using storage_t = sparse_set_paged_t<archetype_global_t, typename base_type::refine_list>;
		using generator_id_t = id_generator_t<base_type::this_id>;

		friend storage_t;

	private:
		static inline uint_t try_dense_id(entity_t ent, bool larger_then_resize)
		{
			uint_t dense_id = sparse_index_global_t<base_type::this_id>::try_dense_id(ent.get_arche(), ent.get_id(), larger_then_resize);
			return dense_id;
		}
		static inline void set_dense_id(entity_t ent, uint_t dense_id)
		{
			sparse_index_global_t<base_type::this_id>::set_dense_id(ent.get_arche(), ent.get_id(), dense_id);
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
			return state_ == entity_state::alive;
		}

		template<typename... Ts>
		static entity_t create_impl(Ts&&...args)
		{
			//entity_t ent = id_generator.get();
			//storage_t* sparse_ptr = try_sparse_set();
			//sparse_ptr->creates(ent, std::forward<Ts>(args)...);
			throw std::exception("archetype_global_t don't allow to create entity by itself.");
			return entity_t::null;
		}

		template<typename... Ts>
		static bool create_or_set_impl(entity_t ent, Ts&&... args)
		{
			storage_t* sparse_ptr = try_sparse_set();
			return sparse_ptr->create_or_set(ent, std::forward<Ts>(args)...);
		}

		template<typename T>
		static T& get_ref_impl(entity_t ent)
		{
			return sparse_set_ptr->get<T>(ent);
		}

		template<typename T>
		static const T& get_impl(entity_t ent)
		{
			return sparse_set_ptr->get<T>(ent);
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
	struct archetype_standard_t final : archetype_base_t<archetype_standard_t<Args...>>
	{
		using this_type = archetype_standard_t<Args...>;
		using base_type = archetype_base_t<archetype_standard_t<Args...>>;
		using storage_t = sparse_set_paged_t<archetype_standard_t, typename base_type::refine_list>;
		using generator_id_t = id_generator_t<base_type::this_id>;

		friend storage_t;

	private:
		static inline uint_t try_dense_id(entity_t ent, bool larger_then_resize)
		{
			uint_t dense_id = sparse_index_standard_t<base_type::this_id>::try_dense_id(ent.get_id(), larger_then_resize);
			return dense_id;
		}
		static inline void set_dense_id(entity_t ent, uint_t dense_id)
		{
			sparse_index_standard_t<base_type::this_id>::set_dense_id(ent.get_id(), dense_id);
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
				}
				spin.unlock();
			}
			return sparse_set_ptr;
		}

		template<typename T>
		inline static void create_or_set_impl_seprate(entity_t ent, T&& arg)
		{
			using global_args = mpl::firstlist_has_t<mpl::to_list_list_t<base_type::global_list>, mpl::traits::remove_qualifier_t<T>>;
			if constexpr(std::is_same_v<global_args, mpl::nulllist>)
			{
				sparse_set_ptr->create_or_set(ent, std::forward<T>(arg));
			}
			else
			{
				//using tglobal = to_archetype_global_t<global_args>;
				constexpr size_t global_index = mpl::indexof_v<global_args, mpl::to_list_list_t<base_type::global_list>>;
				using tglobal = mpl::at_t<global_index, base_type::global_list>;
				tglobal::create_or_set(ent, std::forward<T>(arg));
			}
		}

		template<typename Globals, size_t N>
		inline static void create_or_set_unhandle_globals(entity_t ent)
		{
			if constexpr(N >= mpl::length_v<Globals>)
			{ return; }
			else
			{
				//using tglobal = to_archetype_global_t<mpl::at_t<N, Globals>>;
				using global_args = mpl::at_t<N, Globals>;
				constexpr size_t global_index = mpl::indexof_v<global_args, mpl::to_list_list_t<base_type::global_list>>;
				using tglobal = mpl::at_t<global_index, base_type::global_list>;
				tglobal::create_or_set(ent);
				create_or_set_unhandle_globals<Globals, N + 1>(ent);
			}
		}

		template<typename Globals, size_t N>
		inline static bool remove_impl_global(entity_t ent)
		{
			if constexpr(N >= mpl::length_v<Globals>)
			{ return true; }
			else
			{
				using tglobal = mpl::at_t<N, Globals>;
				bool success = tglobal::remove(ent);
				success &= remove_impl_global<Globals, N + 1>(ent);
				return success;
			}
		}

	public:
		static bool allocated_impl() { return sparse_set_ptr != nullptr; }

		static uint_t available_impl() { return id_generator.available(); }

		template<typename... Ts>
		static entity_t create_impl(Ts&&...args) 
		{
			entity_t ent = id_generator.get();
			create_or_set_impl(ent, std::forward<Ts>(args)...);

			return ent;
		}

		template<typename... Ts>
		static void create_or_set_impl(entity_t ent, Ts&&... args)
		{
			storage_t* sparse_ptr = try_sparse_set();
			using component_globals = mpl::listlist_has_any_of_t<mpl::to_list_list_t<base_type::global_list>, mpl::typelist<Ts...>>;
			if constexpr (mpl::length_v<component_globals> == 0)
			{
				sparse_ptr->create_or_set(ent, std::forward<Ts>(args)...);
			}
			else
			{
				(create_or_set_impl_seprate<Ts>(ent, std::forward<Ts>(args)), ...);
			}
			using unhandle_globals = mpl::remove_any_of_t< mpl::to_list_list_t<base_type::global_list>, mpl::no_duplicate_t< mpl::pairs_values_t<component_globals>>>;
			if constexpr(mpl::length_v<unhandle_globals> > 0)
			{
				create_or_set_unhandle_globals<unhandle_globals, 0>(ent);
			}
		}

		static bool remove_impl(entity_t ent)
		{
			if(nullptr == sparse_set_ptr) [[unlikely]]
			{ return false; }
			bool success = sparse_set_ptr->remove(ent);

			if constexpr(mpl::length_v<base_type::global_list> > 0)
			{
				success &= remove_impl_global<base_type::global_list, 0>(ent);
			}
			return success;
		}

		template<typename T>
		static T& get_ref_impl(entity_t ent)
		{
			return sparse_set_ptr->get<T>(ent);
		}

		template<typename T>
		static const T& get_impl(entity_t ent)
		{
			return sparse_set_ptr->get<T>(ent);
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




