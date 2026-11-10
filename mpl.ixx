export module mpl;

import std;

namespace mpl
{
	namespace traits
	{
		//>>>>>>>> remove_qualifier_t
		template<typename T>
		struct remove_qualifier_type
		{
			using type = std::remove_cvref_t<std::remove_pointer_t<T>>;
		};
		export template<typename T> using remove_qualifier_t = remove_qualifier_type<T>::type;
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

		export template<typename... Args>
		inline constexpr bool is_all_same_v = is_all_same_type<Args...>::value;
		//<<<<<<<<

		//>>>>>>>> if_else_t
		template<bool, typename TrueResult, typename FalseResult> struct if_else_type;

		template<typename TrueResult, typename FalseResult>
		struct if_else_type<true, TrueResult, FalseResult> { using type = TrueResult; };

		template<typename TrueResult, typename FalseResult>
		struct if_else_type<false, TrueResult, FalseResult> { using type = FalseResult; };

		export template<bool Cond, typename TrueResult, typename FalseResult> using if_else_t = if_else_type<Cond, TrueResult, FalseResult>::type;
		//<<<<<<<<

		//>>>>>>>> is_complete_t
		template <typename T, typename = std::void_t<>>
		struct is_complete_t : std::false_type {};

		template <typename T>
		struct is_complete_t<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

		export template <typename T>
		inline constexpr bool is_complete_v = is_complete_t<T>::value;
		//<<<<<<<<
	}
}
