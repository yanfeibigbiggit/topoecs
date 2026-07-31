#include "macros.h"
#include "ankerl/unordered_dense.h"

export module core;
import std;

import typelist;
import configs;

using id_t = unsigned int;
using uint_t = unsigned int;
using byte_t = unsigned char;

using xstring = std::string;
template<typename tkey, typename tvalue> using xmap = ankerl::unordered_dense::map<tkey, tvalue>;

namespace tecs
{
	namespace traits
	{
		//>>>>>>>> is_std_pair_v
		template <typename> struct is_std_pair : std::false_type {};

		template <typename T, typename U> struct is_std_pair<std::pair<T, U>> : std::true_type {};

		MODULE_EXPORT template<typename T> constexpr bool is_std_pair_v = is_std_pair<T>::value;
		//<<<<<<<<

		//>>>>>>>> is_std_string_v
		template<typename> struct is_std_string_type : std::false_type {};

		template<> struct is_std_string_type<xstring> : std::true_type {};
		template<> struct is_std_string_type<const xstring> : std::true_type {};
		template<> struct is_std_string_type<xstring&> : std::true_type {};
		template<> struct is_std_string_type<const xstring&> : std::true_type {};
		template<> struct is_std_string_type<xstring&&> : std::true_type {};
		template<> struct is_std_string_type<const xstring&&> : std::true_type {};

		MODULE_EXPORT template<typename T> constexpr bool is_std_string_v = is_std_string_type<T>::value;
		//<<<<<<<<

		//>>>>>>>> remove_qualifier_t
		template<typename T>
		struct remove_qualifier_type
		{
			using type = std::remove_cvref_t<std::remove_pointer_t<T>>;
		};
		MODULE_EXPORT template<typename T> using remove_qualifier_t = remove_qualifier_type<T>::type;
		//<<<<<<<<

		//>>>>>>>> is_component_name_v
		template<typename CompNameType>
		struct is_component_name_type
		{
			using raw_t = remove_qualifier_t<CompNameType>;
			enum { value = std::is_same_v<raw_t, char> || is_std_string_v<CompNameType> };
		};

		MODULE_EXPORT template<typename T> constexpr bool is_component_name_v = is_component_name_type<T>::value;
		//<<<<<<<<

		//>>>>>>>> is_all_same_v
		template<typename... Args>
		struct is_all_same_type;

		template<> struct is_all_same_type<> { enum { value = 1 }; };

		template<typename First> struct is_all_same_type<First> { enum { value = 1 }; };

		template<typename First, typename Second>
		struct is_all_same_type<First, Second>
		{
			using first_t = First;
			using next_t = Second;
			enum { value = std::is_same_v<First, Second> };
		};

		template<typename First, typename... Args>
		struct is_all_same_type<First, Args...>
		{
			using first_t = First;
			using next_t = is_all_same_type<Args...>::first_t;
			enum { value = std::is_same_v<first_t, next_t>&& is_all_same_type<Args...>::value };
		};

		MODULE_EXPORT template<typename... Args> constexpr bool is_all_same_v = is_all_same_type<Args...>::value;
		//<<<<<<<<

		//>>>>>>>> is_all_component_name_v
		template<typename... Args> struct is_all_component_name_type;

		template<> struct is_all_component_name_type<> { enum { value = 1 }; };

		template<typename First>
		struct is_all_component_name_type<First>
		{
			using first_t = First;
			enum { value = traits::is_component_name_v<First> };
		};

		template<typename First, typename... Args>
		struct is_all_component_name_type<First, Args...>
		{
			using first_t = First;
			enum { value = traits::is_component_name_v<first_t> && is_all_component_name_type<Args...>::value };
		};

		MODULE_EXPORT template<typename... Args> constexpr bool is_all_component_name_v = is_all_component_name_type<Args...>::value;
		//<<<<<<<<

		//>>>>>>>> select_if_else_t
		template<bool, typename TrueResult, typename FalseResult> struct select_if_else_type;

		template<typename TrueResult, typename FalseResult>
		struct select_if_else_type<true, TrueResult, FalseResult> { using type = TrueResult; };

		template<typename TrueResult, typename FalseResult>
		struct select_if_else_type<false, TrueResult, FalseResult> { using type = FalseResult; };

		MODULE_EXPORT template<bool Cond, typename TrueResult, typename FalseResult> using select_if_else_t = select_if_else_type<Cond, TrueResult, FalseResult>::type;
		//<<<<<<<<

	}

	namespace must
	{
		template<typename T> concept is_component_name_t = traits::is_component_name_v<T>;
	}

	namespace convert
	{
		template<typename StringType>
			requires must::is_component_name_t<StringType>
		const char* cast_component_name(StringType&& comp_name)
		{
			if constexpr (traits::is_std_string_v<StringType>)
			{
				return comp_name.c_str();
			}
			else
			{
				return comp_name;
			}
		}
	}

	namespace misc
	{
		//////////////////////////////////////////////////////////
		//type_id_counter_t
		//////////////////////////////////////////////////////////
		template<typename UniqueTag>
		struct type_id_counter_t
		{
			template<std::size_t N>
			struct const_counter_flag
			{
				//return must be `auto` for requires statement in const_counter_next()
				friend constexpr auto adl_const_counter_flag(const_counter_flag<N>);
			};

			template<std::size_t N>
			struct const_counter_writer
			{
				//return must be `auto` for requires statement in const_counter_next()
				friend constexpr auto adl_const_counter_flag(const_counter_flag<N>)
				{
					return N;
				}
				constexpr void nothing() {}
			};

			template<std::size_t N = 0, auto uique = []() {} >
			static constexpr std::size_t const_counter_next()
			{
				constexpr bool defined = requires{ adl_const_counter_flag(const_counter_flag<N>{}); };
				if constexpr (!defined)
				{
					const_counter_writer<N> writer;
					writer.nothing();
					return N;
				}
				else
				{
					return const_counter_next<N + 1>();
				}
			}

			template<typename ... Args>
			struct type_id_t
			{
				static constexpr std::size_t value = const_counter_next();
			};
		};

		struct type_id_gobal_tag_t {};
		template<typename... Args>
		constexpr std::size_t type_id_global = type_id_counter_t< type_id_gobal_tag_t>::type_id_t<Args...>::value;

		struct type_id_component_tag_t {};
		export template<typename T>
		constexpr std::size_t type_id_component = misc::type_id_counter_t< type_id_component_tag_t>::type_id_t<T>::value;

		struct type_id_archetype_tag_t {};
		export template<typename T>
		constexpr std::size_t type_id_archetype = misc::type_id_counter_t< type_id_archetype_tag_t>::type_id_t<T>::value;


		//////////////////////////////////////////////////////////
		//id_type_t
		//////////////////////////////////////////////////////////
		template<std::size_t>
		struct id_type_t
		{

		};


		//////////////////////////////////////////////////////////
		//type_name_id
		//////////////////////////////////////////////////////////
		template <typename T>
		constexpr const char* type_name_id()
		{
			const char* str;
#if defined(__clang__) || defined(__GNUC__)
			str = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			str = __FUNCSIG__;
#else
			str = char unknown compiler[-1];
#endif

			return str;
		}

		constexpr int compare_type_name_id(const char* s1, const char* s2)
		{
			//std::strlen and sizeof is not constexpr
			int skip = 0;
#if defined(_MSC_VER)
			skip = 47;
#endif
			for (int i = skip; ; i++)
			{
				auto c1 = s1[i]; auto c2 = s2[i];
				if (c1 == '\0' && c2 == '\0') { return 0; }
				if (c1 == c2) { continue; }
				if (c1 == '\0') { return 0; }
				if (c2 == '\0') { return 1; }
				return c1 - c2 < 0;
			}
			return 0;
		}

		//////////////////////////////////////////////////////////
		//sort_index_sequence
		//////////////////////////////////////////////////////////
		template<auto Array, size_t... I>
		constexpr auto constexpr_array_to_index_sequence(std::index_sequence<I...>)
		{
			return std::index_sequence<Array[I]...>{};
		}

		template<typename... Args>
		constexpr auto sort_index_sequence()
		{
			constexpr std::size_t halfbits = sizeof(std::size_t) * 4;
			constexpr std::size_t N = sizeof...(Args);
			std::array<std::size_t, N> type_array{ (type_id_component<Args> << halfbits)... };
			for (int i = 0; i < N; i++)
			{
				type_array[i] += i;
			}

			for (int i = 0; i < N - 1; i++)
			{
				for (int j = 0; j < N - 1 - i; j++)
				{
					if(type_array[j] > type_array[j + 1])
					{
						auto temp = type_array[j + 1];
						type_array[j + 1] = type_array[j];
						type_array[j] = temp;
					}
				}
			}

			for (int i = 0; i < N; i++)
			{
				type_array[i] <<= halfbits;
				type_array[i] >>= halfbits;
			}
			return type_array;
		}

		//////////////////////////////////////////////////////////
		//data_size_of_t
		//////////////////////////////////////////////////////////
		template<typename... Args> struct data_size_of_t;

		template<typename T, typename... Args>
		struct data_size_of_t<T, Args...>
		{
			enum { value = sizeof(T) + data_size_of_t<Args...>::value };
		};

		template<typename T>
		struct data_size_of_t<T>
		{
			enum { value = sizeof(T) };
		};

		//////////////////////////////////////////////////////////
		//type_map_t
		//////////////////////////////////////////////////////////
		export template<typename TKey, std::size_t MAX = configs::ARCHETYPE_TYPE_MAX_COUNT>
		struct type_map_t
		{
		private:
			template<std::size_t N>
			struct type_map_flag
			{
				//return must be `auto` for requires statement in push_type_pair()
				friend constexpr auto adl_type_map_flag(type_map_flag<N>);
			};

			template<std::size_t N>
			struct type_map_writer
			{
				//return must be `auto` for requires statement in push_type_pair()
				friend constexpr auto adl_type_map_flag(type_map_flag<N>)
				{
					return N;
				}
				consteval void nothing() {}
			};
		public:
			template<std::size_t N>
			[[nodiscard]] static consteval std::size_t push()
			{ 
				type_map_writer<N> writer;
				writer.nothing();
				return N;
			}

			template<std::size_t N>
			static constexpr bool has()
			{
				constexpr auto x = N;
				for (std::size_t i = 0; i < MAX; i++)
				{
					constexpr bool defined = requires{ adl_type_map_flag(type_map_flag<N>{}); };
					if (defined) 
					{
						return true; 
					}
				}
				return false;
			}
		};

		//////////////////////////////////////////////////////////
		//map_component_archetype
		//////////////////////////////////////////////////////////
		template<std::size_t ID, typename... TList> struct map_component_archetype;

		template<std::size_t ID, typename T, typename... TList>
		struct map_component_archetype<ID, mpl::typelist<T, TList...>> : map_component_archetype<ID, mpl::typelist<TList...>>
		{
			enum {value = type_map_t<T>::template push<ID>()};
		};

		template<std::size_t ID>
		struct map_component_archetype<ID, mpl::nulllist > { enum { value = std::numeric_limits<std::size_t>::max() }; };
	}

	namespace maths
	{
		constexpr std::size_t round_power_2(std::size_t v)
		{
			v = v <= 0 ? 1 : v;
			int x = 1;
			while (x <= v) { x *= 2; }
			return (x - v < v - x / 2) ? x : x / 2;
		}
	}
}

namespace tecs
{
	class spinlock_t
	{
	public:
		std::atomic_flag flag;

		void lock(const std::memory_order order_ = std::memory_order::memory_order_seq_cst)
		{
			while (flag.test_and_set(order_));
		}
		void unlock()
		{
			flag.clear();
		}
	};
}

namespace tecs
{
	const id_t MASK_ID		= 0x00ffffff;
	const id_t MASK_VERSION	= 0xff000000;
	const id_t VERSION_ONE = 1 << 24;

	MODULE_EXPORT const id_t ID_NULL = 0x00ffffff;

	inline uint_t get_id(id_t e) { return e & MASK_ID; }
	inline id_t set_id(id_t e, uint_t newid) { return e - get_id(e) + newid; }
	inline id_t get_version(id_t e) { return (e & MASK_VERSION) >> 24; }
	inline id_t set_version(id_t e, id_t v) { return get_id(e) + (v << 24); }
	inline id_t incr_version(id_t e) { return e + VERSION_ONE; }

	enum error_code_t { e_null_pointer, e_negative_integer, e_missing_component };
	

	MODULE_EXPORT class entity_t
	{
	private:
		id_t id_v = 0;
	public:
		static entity_t null;
	public:
		entity_t() {}
		entity_t(uint_t id_, uint_t version_)
		{
			id_v = set_version(id_, version_);
		}

		uint_t id() { return get_id(id_v); }
		uint_t version() { return get_version(id_v); }
	};
	entity_t entity_t::null = { ID_NULL, 0 };

	class id_generator_t
	{
	private:
		id_t current;
		std::queue<id_t> free;

	public:
		id_generator_t()
		{
			current = 0;
		}

		id_t get()
		{
			id_t entity = 0;
			if (!free.empty())
			{
				entity = free.front();
				free.pop();
			}
			else
			{
				if (current >= ID_NULL)
				{
					//throw
					return ID_NULL;
				}
				entity = current;
				current += 1;
			}
			return entity;
		}

		void recycle(entity_t e)
		{
			id_t id_v = incr_version(e.id());
			free.push(id_v);
		}
	};


	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	export template<typename... Args>
	struct archetype_storage_t
	{
		using this_type = archetype_storage_t<Args...>;
		static constexpr std::size_t this_id = misc::type_id_archetype<this_type>;
		
		static constexpr std::size_t COUNT_PER_CHUNK = 
			maths::round_power_2(configs::ARCHETYPE_CHUNK_LENGTH / (misc::data_size_of_t<Args...>::value + sizeof(entity_t)));

		enum { map_component_archetype = misc::map_component_archetype<this_id, mpl::typelist<Args...>>::value };

		template<typename T>
		struct column_t
		{
			using container_t = std::array<T, COUNT_PER_CHUNK>;
			container_t values;
			
			T& operator[](std::size_t index)
			{
				return values[index];
			}
			const T& operator[](std::size_t index) const
			{
				return values[index];
			}
			//container_t::iterator begin() noexcept {
			//	return values.begin();
			//}

			//container_t::const_iterator end() const noexcept {
			//	return values.end();
			//}
		};

		struct chunk_t : column_t<Args>...
		{
			column_t<entity_t> entities;

			template<typename T>
			auto& column()
			{
				return column_t<T>::values;
			}

			template<typename T>
			void set(T t)
			{
				column_t<T>::operator[](0) = t;
			}
		};

		struct sparse_set_t
		{

		};

	private:
		static chunk_t columns;

	public:
		static void name() { std::cout << misc::type_name_id<this_type>() << std::endl; }

		template<typename... Ts>
		static void add(Ts...args) 
		{
			
		}

		template<typename T>
		static void set(T t)
		{
			//auto& a = columns.column<T>();
			//a[0] = t;
			columns.set(t);
		}
		template<typename T>
		static T get()
		{
			const auto& b = columns.column<T>();
			return b[0];
		}
	};
	template<typename... Args> typename archetype_storage_t<Args...>::chunk_t archetype_storage_t<Args...>::columns;

	template<typename... Args>
	struct archetype_define_t
	{
	private:
		static constexpr auto apply()
		{
			constexpr auto storage = apply_impl(
				misc::constexpr_array_to_index_sequence< misc::sort_index_sequence<Args...>() >(
					std::make_index_sequence<sizeof...(Args)>{}));

			return storage;
		}

		template<size_t... I>
		static constexpr auto apply_impl(std::index_sequence<I...>)
		{
			using origin_sequence_t = mpl::typelist<Args...>;
			using type = archetype_storage_t<mpl::at<I, origin_sequence_t>::type...>;
			return type{};
		}

	public:
		using type = decltype(apply());
	};

	export template<typename... Args>
	using archetype_t = archetype_define_t<Args...>::type;

	template<typename... Args>
	struct query_t
	{

	};

	MODULE_EXPORT template<typename UniqueTag>
	struct world_t
	{

	};

}