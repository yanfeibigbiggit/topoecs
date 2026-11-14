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
template<typename T, size_t N> using xarray = std::array<T, N>;
template<typename T> using xvector = std::vector<T>;

namespace tecs
{
	//64 bit layout: [UNUSED | VERSION_BITS | ARCHETYPE_BITS | ID_BITS]
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
		uint_t flag		: 32 - ARCHETYPE_BITS;
		uint_t ver		: VERSION_BITS;
		uint_t arche    : ARCHETYPE_BITS;
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

	enum entity_state
	{
		miss,
		outdate,
		alive,
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args> struct sparse_set_default_t;
	template<typename... Args>
	struct sparse_set_default_t<mpl::typelist<Args...>>
	{
		static constexpr size_t ROW_SIZE = (utility::data_size_of_t<Args...>::value + sizeof(entity_t) + sizeof(uint_t));

		static constexpr size_t decide_element_per_chunk()
		{
			size_t count = maths::round_power_2(configs::ARCHETYPE_DEFAULT_CHUNK_MEM_SIZE / ROW_SIZE);
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

			//T& operator[](uint_t index) { return values[index]; }
			//const T& operator[](uint_t index) const { return values[index]; }
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
			inline T& get(uint_t dense_index)
			{
				auto& container = column<T>();
				return container[dense_index];
			}
			template<typename T>
			inline const T& get(uint_t dense_index) const
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
		public:

			template<typename T>
			inline T& get(entity_t ent)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{
					throw std::exception(std::format("miss entity: id:{} ver:{} arche:{}", ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
				}
				return get<T>(dense_index);
			}
			template<typename T>
			inline const T& get(entity_t ent) const
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL = dense_index)
				{
					throw std::exception(std::format("miss entity: id:{} ver:{} arche:{}", ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
				}
				return get<T>(dense_index);
			}

			inline bool has_entity(entity_t ent)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{ return false; }
				const auto& latest = get<entity_t>(dense_index);
				return ent.is_same(latest);
			}

			template<typename... Ts>
			inline bool creates(entity_t ent, Ts&&...args)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL != dense_index)
				{ return false; }

				dense_index = length;
				indexies[sparse_index] = length;
				length++;
				((set<Ts>(dense_index, std::forward<Ts>(args))), ...);
				set<entity_t>(dense_index, std::move(ent));
				return true;
			}

			template<typename... Ts>
			inline bool create_or_set(entity_t ent, Ts&&...args)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{
					dense_index = length;
					indexies[sparse_index] = length;
					length++;
					((set<Ts>(dense_index, std::forward<Ts>(args))), ...);
					set<entity_t>(dense_index, std::move(ent));
					return true;
				}

				entity_t& latest = get<entity_t>(dense_index);
				//must be same entity
				if (!latest.is_same(ent))
				{ return false; }

				((set<Ts>(dense_index, std::forward<Ts>(args))), ...);

				return true;
			}

			template<typename... Ts>
			inline entity_state sets(entity_t ent, Ts&&...args)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				if (SPARSE_INDEX_NULL == dense_index)
				{ return miss; }

				entity_t& target_ent = get<entity_t>(dense_index);
				//must be same entity
				if (!target_ent.is_same(ent))
				{ return outdate; }

				((set<Ts>(dense_index, std::forward<Ts>(args))), ...);

				return alive;
			}

			bool remove(entity_t ent)
			{
				uint_t sparse_index = ent.sparse_id<COUNT_PER_CHUNK>();
				uint_t dense_index = indexies[sparse_index];
				//has been deleted or not been alloc
				if (SPARSE_INDEX_NULL == dense_index) [[unlikely]]
				{
					return false;
				}
				entity_t& target_ent = get<entity_t>(dense_index);
				//must be same entity
				if (!target_ent.is_same(ent))
				{
					return false;
				}
				entity_t& last_ent = get<entity_t>(length - 1);
				uint_t last_sparse_index = last_ent.sparse_id< COUNT_PER_CHUNK>();
				((set<Args>(dense_index, std::forward<Args>(get<Args>(length - 1)))), ...);
				indexies[last_sparse_index] = dense_index;
				indexies[sparse_index] = SPARSE_INDEX_NULL;
				length--;

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
		// begin sparse_set_default_t main declare
		////////////////////////////////////////////////////////////////////////////////////////////////

		using chunk_ptrs_t = xvector<chunk_t*>;
		using indexies_t = xvector<uint_t>;

	private:
		chunk_ptrs_t chunks;
		indexies_t indexies;
		uint_t length = 0;

	public:
		sparse_set_default_t()
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
			{
				return nullptr;
			}
			return chunks[dense_id];
		}

		inline bool has_entity(entity_t ent)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			chunk_t* chunk = get_chunk(chunk_id);
			if(nullptr == chunk)
			{ return false; }
			return chunk->has_entity(ent);
		}
		template<typename... Ts>
		inline void creates(entity_t ent, Ts&&... args)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			chunk_t* chunk = try_get_chunk(chunk_id);
			chunk->creates(ent, std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		inline void create_or_set(entity_t ent, Ts&&... args)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			chunk_t* chunk = get_chunk(chunk_id);
			if(nullptr == chunk)
			{
				creates(ent, std::forward<Ts>(args)...);
			}
			else
			{
				chunk->sets(ent, std::forward<Ts>(args)...);
			}
		}

		template<typename... Ts>
		inline bool sets(entity_t ent, Ts&&...args)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			bool ok = chunk->sets(ent, std::forward<Ts>(args)...);
			return ok;
		}

		template<typename T>
		inline T& get(entity_t ent)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			if(nullptr == chunk)
			{
				throw std::exception(std::format("miss entity: id:{} ver:{} arche:{}", ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
			}
			return chunk->get<T>(ent);
		}

		template<typename T>
		inline const T& get(entity_t ent) const
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			if (nullptr == chunk)
			{
				throw std::exception(std::format("miss entity: id:{} ver:{} arche:{}", ent.get_id(), ent.get_version(), ent.get_arche()).c_str());
			}
			return chunk->get<T>(ent);
		}

		bool remove(entity_t ent)
		{
			auto chunk_id = ent.chunk_id<COUNT_PER_CHUNK>();
			auto chunk = get_chunk(chunk_id);
			if (nullptr == chunk) [[unlikely]]
			{
				return false;
			}
			bool ok = chunk->remove(ent);
			return ok;
		}


		template<typename Lambda, typename... Ts>
		void each(Lambda&& lambda) noexcept
		{
			for (uint_t i = 0; i < length; i++)
			{
				chunks[i]->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
			}
		}

		//chunk_ptrs_t::iterator begin() noexcept { return chunks.begin(); }

		//chunk_ptrs_t::const_iterator end() const noexcept { return chunks.begin() + length; }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename Derived> struct archetype_base_t;

	export template <class C>
		concept IsDerivedArchetypeConcept = requires(C c)
	{
		// IILE
		[] <typename... X>(archetype_base_t<X...>&) {}(c);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args>
	struct sort_component_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_unpack_t<Args...>>;

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
	template<typename T, typename = void> struct archetype_arg_list_helper { using type = mpl::nulllist; };
	template<typename T>
	struct archetype_arg_list_helper<T, std::void_t<decltype(sizeof(T::order_list))>>
	{
		using type = T::order_list;
	};

	template<typename... TList> struct archetype_arg_list_type;
	template<typename T, typename... TList>
	struct archetype_arg_list_type<mpl::typelist<T, TList...>>
	{
		using order_list = mpl::traits::if_else_t<
										!IsDerivedArchetypeConcept<T>,
										mpl::concat_t<T, typename archetype_arg_list_type<mpl::typelist<TList...>>::order_list>,
										mpl::concat_t<typename archetype_arg_list_helper<T>::type, typename archetype_arg_list_type<mpl::typelist<TList...>>::order_list>>;
	};
	template<>
	struct archetype_arg_list_type<mpl::nulllist>
	{
		using order_list = mpl::nulllist;
	};

	template<typename TList> using archetype_arg_list_t = typename archetype_arg_list_type<TList>::order_list;

	////////////////////////////////////////////////////////////////////////////////////////////////
	export template<typename Derived> struct archetype_base_t { };

	////////////////////////////////////////////////////////////////////////////////////////////////
	export template<typename... Args>
	struct archetype_default_t : archetype_base_t<archetype_default_t<Args...>>
	{
		using order_list = mpl::typelist<Args...>;
		static inline constexpr size_t this_id = misc::id_by_archetype<archetype_default_t>;
	private:
		enum { map_component_archetype = misc::map_component_archetype<this_id, order_list>::value };

		using generator_id_t = id_generator_t<this_id>;
		using storage_t = sparse_set_default_t<order_list>;
		friend storage_t;

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

		template<typename Lambda, typename... Ts>
		static void each_impl(Lambda&& lambda, mpl::typelist<Ts...>) noexcept
		{
			static_assert(!mpl::has_raw_reference_of_v<mpl::typelist<Ts...>, entity_t>, "each() function: forbidden reference to entity_t");
			if (!allocated()) [[unlikely]]
			{ return; }

			sparse_set_ptr->each<Lambda, Ts...>(std::forward<Lambda>(lambda));
		}

	public:
		static bool allocated() noexcept { return sparse_set_ptr != nullptr; }

		static uint_t available() noexcept { return id_generator.available(); }

		static bool has_entity(entity_t ent) noexcept
		{
			if (!allocated()) [[unlikely]]
			{
				return false;
			}

			entity_state state_ = sparse_set_ptr->has_entity(ent);
			return state_ == entity_state::alive;
		}

		template<typename... Ts>
		static entity_t create(Ts&&...args)
		{
			entity_t ent = id_generator.get();
			storage_t* sparse_ptr = try_sparse_set();
			sparse_ptr->creates(ent, std::forward<Ts>(args)...);
			return ent;
		}

		template<typename... Ts>
		static bool create_or_set(entity_t ent, Ts&&... args)
		{
			storage_t* sparse_ptr = try_sparse_set();
			return sparse_ptr->create_or_set(ent, std::forward<Ts>(args)...);
		}

		//template<typename T>
		//static T& get_ref(entity_t ent)
		//{
		//	return sparse_set_ptr->get<T>(ent);
		//}

		template<typename T>
		static const T& get(entity_t ent)
		{
			return sparse_set_ptr->get<T>(ent);
		}

		static bool remove(entity_t ent)
		{
			if (!allocated()) [[unlikely]]
			{
				return false;
			}
			bool ok = sparse_set_ptr->remove(ent);
			return ok;
		}

		template<typename... Ts>
		static void each(void(*func)(Ts...)) noexcept
		{
			static_assert(mpl::is_sub_v<order_list, mpl::list_remove_qualifier_t<mpl::typelist<Ts...>>>, "args of lambda don't match archetype");
			each_impl(func, mpl::typelist<Ts...>{});
		}

		template<typename Lambda>
		static void each(Lambda&& lambda) noexcept
		{
			using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
			static_assert(mpl::is_sub_v<order_list, mpl::list_remove_qualifier_t<args_type>>, "args of lambda don't match archetype");
			each_impl(std::forward<Lambda>(lambda), args_type{});
		}
	};
	template<typename... Args> archetype_default_t<Args...>::storage_t* archetype_default_t<Args...>::sparse_set_ptr = nullptr;
	template<typename... Args> archetype_default_t<Args...>::generator_id_t  archetype_default_t<Args...>::id_generator{};

	////////////////////////////////////////////////////////////////////////////////////////////////
	export template<typename... Args>
	struct archetype_t
	{
		using arg_list = archetype_arg_list_t<mpl::typelist<entity_t, Args...>>;
		using order_list = sort_component_t<arg_list>::type;

		template<typename... Ts> struct to_default_t;
		template<typename... Ts>
		struct to_default_t<mpl::typelist<Ts...>>
		{
			using type = archetype_default_t<Ts...>;
		};
		using default_t = to_default_t<order_list>::type;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////

	export template <class C>
	concept HasEachMethod = std::is_function_v<decltype(C::each)>;

	export template <class C>
	concept HasRunMethod = std::is_function_v<decltype(C::run)>;

	export template<class C> concept IsTemplateClassConcept = requires(C c)
	{
		[] <template<class...> class T, class... Ts>(T<Ts...>) {}(c);
	};

	export struct behaviour_base_t {};
	export template <class C>
	concept IsDerivedBehaviourConcept = std::is_base_of_v<behaviour_base_t, C>;

	////////////////////////////////////////////////////////////////////////////////////////////////
}




