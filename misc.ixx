export module tecs.basic:misc;

import std;
import typelist;
import configs;

namespace tecs
{
	/// <summary>
	/// Do not export `misc` to any user modules; it is intended solely for internal use within `tecs`. 
	/// Doing so would cause conflicts in component and archetype registration.
	/// </summary>
	namespace misc
	{
		using size_t = std::size_t;
		////////////////////////////////////////////////////////////
		////type_id_counter_t
		////////////////////////////////////////////////////////////
		template<typename UniqueTag>
		struct type_id_counter_t
		{
			template<size_t N>
			struct const_counter_flag
			{
				//return must be `auto` for requires statement in const_counter_next()
				friend constexpr auto adl_const_counter_flag(const_counter_flag<N>);
			};

			template<size_t N>
			struct const_counter_writer
			{
				//return must be `auto` for requires statement in const_counter_next()
				friend constexpr auto adl_const_counter_flag(const_counter_flag<N>)
				{
					return N;
				}
				constexpr void nothing() {}
			};

			template<size_t N = configs::TYPE_ID_START_INDEX, auto uique = []() {} >
			static constexpr size_t const_counter_next()
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
				static inline constexpr size_t value = const_counter_next();
			};
		};

		struct type_id_gobal_tag_t {};
		template<typename... Args>
		constexpr size_t type_id_global = type_id_counter_t< type_id_gobal_tag_t>::type_id_t<Args...>::value;

		struct type_id_component_tag_t {};
		export template<typename T>
		constexpr size_t type_id_component = type_id_counter_t< type_id_component_tag_t>::type_id_t<T>::value;

		struct type_id_archetype_tag_t {};
		export template<typename T>
		constexpr size_t type_id_archetype = type_id_counter_t< type_id_archetype_tag_t>::type_id_t<T>::value;


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
			constexpr size_t halfbits = sizeof(size_t) * 4;
			constexpr size_t N = sizeof...(Args);
			std::array<size_t, N> type_array{ (type_id_component<Args> << halfbits)... };
			for (int i = 0; i < N; i++)
			{
				type_array[i] += i;
			}

			for (int i = 0; i < N - 1; i++)
			{
				for (int j = 0; j < N - 1 - i; j++)
				{
					if (type_array[j] > type_array[j + 1])
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

		template<typename... Params, template<typename...> class T>
		constexpr auto sort_index_sequence(T<Params...>)
		{
			return sort_index_sequence<Params...>();
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
		export template<typename TKey, size_t MAX = configs::ARCHETYPES_MAX>
		struct type_map_t
		{
		private:
			template<size_t N>
			struct type_map_flag
			{
				//return must be `auto` for requires statement in push()
				friend constexpr auto adl_type_map_flag(type_map_flag<N>);
			};

			template<size_t N>
			struct type_map_writer
			{
				//return must be `auto` for requires statement in push()
				friend constexpr auto adl_type_map_flag(type_map_flag<N>)
				{
					return N;
				}
				consteval void nothing() {}
			};
		public:
			template<size_t N>
			static consteval size_t push()
			{
				type_map_writer<N> writer;
				writer.nothing();
				return N;
			}

			template<size_t N>
			static constexpr bool has()
			{
				constexpr auto x = N;
				for (size_t i = 0; i < MAX; i++)
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
		template<size_t ID, typename... TList> struct map_component_archetype;

		template<size_t ID, typename T, typename... TList>
		struct map_component_archetype<ID, mpl::typelist<T, TList...>> : map_component_archetype<ID, mpl::typelist<TList...>>
		{
			enum { value = type_map_t<T>::template push<ID>() };
		};

		template<size_t ID>
		struct map_component_archetype<ID, mpl::nulllist > { enum { value = std::numeric_limits<size_t>::max() }; };
	}


	//namespace misc
	//{
	//	//////////////////////////////////////////////////////////
	//	// https://www.reddit.com/r/cpp/comments/6ydxya/comment/dmmoveu/
	//	//////////////////////////////////////////////////////////
	//	template <typename TAG, int N>
	//	struct tag {
	//		friend auto loophole(tag<TAG, N>);
	//		constexpr friend int cloophole(tag<TAG, N>);
	//	};

	//	/*
	//	  The definitions of friend functions.
	//	*/
	//	template <typename TAG, typename U, int N, bool B>
	//	struct fn_def {
	//		friend auto loophole(tag<TAG, N>) { return U{}; }
	//		constexpr friend int cloophole(tag<TAG, N>) { return 0; }
	//	};

	//	/*
	//	  This specialization is to avoid multiple definition errors.
	//	*/
	//	template <typename TAG, typename U, int N>
	//	struct fn_def<TAG, U, N, true> {};

	//	/*
	//	  This has a templated conversion operator which in turn triggers instantiations.
	//	  Important point, using sizeof seems to be more reliable. Also default template
	//	  arguments are "cached" (I think). To fix that I provide a U template parameter to
	//	  the ins functions which do the detection using constexpr friend functions and SFINAE.
	//	*/
	//	template <typename TAG, int N>
	//	struct c_op {
	//		template <typename U, int M>
	//		static auto ins(...) -> int;
	//		template <typename U, int M, int = cloophole(tag<TAG, M>{}) >
	//		static auto ins(int) -> char;

	//		template <typename U, int = sizeof(fn_def<TAG, U, N, sizeof(ins<U, N>(0)) == sizeof(char)>)>
	//		operator U* ();
	//	};

	//	export template <typename T, typename TAG = void, int N = 1,
	//		bool b = (sizeof(c_op<TAG, N>::template ins<T, N>(0)) == sizeof(char))>
	//		struct tid {
	//		const static int value = tid<T, TAG, N + 1>::value;
	//	};

	//	export template <typename T, typename TAG, int N>
	//		struct tid<T, TAG, N, false> {
	//		typedef decltype(static_cast<T*>(c_op<TAG, N>{}), 0) X;
	//		const static int value = N;
	//	};

	//	export template <int N, typename TAG = void>
	//		struct type_by_id {
	//		typedef decltype(loophole(tag<TAG, N>())) type;
	//	};
	//}
}

