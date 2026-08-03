export module mpl;

// Should we use [[msvc::known_semantics]] to tell the compiler that certain
// type trait specializations have the standard-mandated semantics?
#if _HAS_MSVC_ATTRIBUTE(known_semantics)
#define _MSVC_KNOWN_SEMANTICS [[msvc::known_semantics]]
#else
#define _MSVC_KNOWN_SEMANTICS
#endif

namespace mpl
{
	export template <class _Ty>
	struct remove_const { // remove top-level const qualifier
	    using type = _Ty;
	};

	template <class _Ty>
	struct remove_const<const _Ty> {
	    using type = _Ty;
	};

	export template <class _Ty>
	    using remove_const_t = typename remove_const<_Ty>::type;

	export template <class _Ty>
	    struct remove_volatile { // remove top-level volatile qualifier
	    using type = _Ty;
	};

	template <class _Ty>
	struct remove_volatile<volatile _Ty> {
	    using type = _Ty;
	};

	export template <class _Ty>
	    using remove_volatile_t = typename remove_volatile<_Ty>::type;

	export template <class _Ty>
	    struct remove_cv { // remove top-level const and volatile qualifiers
	    using type = _Ty;

	    template <template <class> class _Fn>
	    using _Apply = _Fn<_Ty>; // apply cv-qualifiers from the class template argument to _Fn<_Ty>
	};

	template <class _Ty>
	struct remove_cv<const _Ty> {
	    using type = _Ty;

	    template <template <class> class _Fn>
	    using _Apply = const _Fn<_Ty>;
	};

	template <class _Ty>
	struct remove_cv<volatile _Ty> {
	    using type = _Ty;

	    template <template <class> class _Fn>
	    using _Apply = volatile _Fn<_Ty>;
	};

	template <class _Ty>
	struct remove_cv<const volatile _Ty> {
	using type = _Ty;

	    template <template <class> class _Fn>
	    using _Apply = const volatile _Fn<_Ty>;
	};

	export template <class _Ty>
	using remove_cv_t = typename remove_cv<_Ty>::type;

	export template <class _Ty>
		struct remove_extent { // remove array extent
		using type = _Ty;
	};

	template <class _Ty, size_t _Ix>
	struct remove_extent<_Ty[_Ix]> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_extent<_Ty[]> {
		using type = _Ty;
	};

	export template <class _Ty>
		using remove_extent_t = typename remove_extent<_Ty>::type;

	export template <class _Ty>
		struct remove_all_extents { // remove all array extents
		using type = _Ty;
	};

	template <class _Ty, size_t _Ix>
	struct remove_all_extents<_Ty[_Ix]> {
		using type = typename remove_all_extents<_Ty>::type;
	};

	template <class _Ty>
	struct remove_all_extents<_Ty[]> {
		using type = typename remove_all_extents<_Ty>::type;
	};

	export template <class _Ty>
		using remove_all_extents_t = typename remove_all_extents<_Ty>::type;

	export template <class _Ty>
		struct remove_pointer {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_pointer<_Ty*> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_pointer<_Ty* const> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_pointer<_Ty* volatile> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_pointer<_Ty* const volatile> {
		using type = _Ty;
	};

	export template <class _Ty>
	using remove_pointer_t = typename remove_pointer<_Ty>::type;

	export template <class _Ty>
		struct remove_reference {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&;
	};

	template <class _Ty>
	struct remove_reference<_Ty&&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&&;
	};

	export template <class _Ty>
		using remove_reference_t = typename remove_reference<_Ty>::type;

	template <class _Ty>
	using _Const_thru_ref = typename remove_reference<_Ty>::_Const_thru_ref_type;

	template <class _Ty>
	using _Remove_cvref_t _MSVC_KNOWN_SEMANTICS = remove_cv_t<remove_reference_t<_Ty>>;

	export template <class _Ty>
		using remove_cvref_t = _Remove_cvref_t<_Ty>;

	export template <class _Ty>
		struct remove_cvref {
		using type = remove_cvref_t<_Ty>;
	};

}

namespace mpl
{
	export template <class _Ty, _Ty _Val>
	struct integral_constant {
		static constexpr _Ty value = _Val;

		using value_type = _Ty;
		using type = integral_constant;

		constexpr operator value_type() const noexcept {
			return value;
		}

		[[nodiscard]] constexpr value_type operator()() const noexcept {
			return value;
		}
	};

	export template <bool _Val>
	using bool_constant = integral_constant<bool, _Val>;

	export using true_type = bool_constant<true>;
	export using false_type = bool_constant<false>;

	export template <bool _Test, class _Ty = void>
	struct enable_if {}; // no member "type" when !_Test

	template <class _Ty>
	struct enable_if<true, _Ty> { // type is _Ty for _Test
		using type = _Ty;
	};

	export template <bool _Test, class _Ty = void>
	using enable_if_t = typename enable_if<_Test, _Ty>::type;

	export template <bool _Test, class _Ty1, class _Ty2>
	struct conditional { // Choose _Ty1 if _Test is true, and _Ty2 otherwise
		using type = _Ty1;
	};

	template <class _Ty1, class _Ty2>
	struct conditional<false, _Ty1, _Ty2> {
		using type = _Ty2;
	};

	export template <bool _Test, class _Ty1, class _Ty2>
	using conditional_t = typename conditional<_Test, _Ty1, _Ty2>::type;

    ///////////////////////////////////////////////////////////////
    export template <class, class> constexpr bool is_same_v = false;
    template <class _Ty> constexpr bool is_same_v<_Ty, _Ty> = true;

    ///////////////////////////////////////////////////////////////
    export template <class _Ty, class... _Types> constexpr bool is_any_of_v = (is_same_v<_Ty, _Types> || ...);

    ///////////////////////////////////////////////////////////////
    export template <class _Ty>
    constexpr bool is_integral_v = is_any_of_v<remove_cv_t<_Ty>, bool, char, signed char, unsigned char, wchar_t,
#ifdef __cpp_char8_t
        char8_t,
#endif // defined(__cpp_char8_t)
        char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

	export template <class _Ty>
	constexpr bool is_floating_point_v = is_any_of_v<remove_cv_t<_Ty>, float, double, long double>;

	export template <class _Ty>
	struct is_floating_point : bool_constant<is_floating_point_v<_Ty>> {};

	export template <class _Ty>
	constexpr bool is_arithmetic_v = // determine whether _Ty is an arithmetic type
		is_integral_v<_Ty> || is_floating_point_v<_Ty>;

	export template <class _Ty>
	struct is_arithmetic : bool_constant<is_arithmetic_v<_Ty>> {};

	export template <class _Ty>
	constexpr bool is_void_v = is_same_v<remove_cv_t<_Ty>, void>;

	export template <class _Ty>
	struct is_void : bool_constant<is_void_v<_Ty>> {};

	export template <class... _Types>
	using void_t = void;

    ///////////////////////////////////////////////////////////////
    export template <class _Ty, _Ty... _Vals>
        struct integer_sequence
    { // sequence of integer parameters
        static_assert(is_integral_v<_Ty>, "integer_sequence<T, I...> requires T to be an integral type.");

        using value_type = _Ty;

        static constexpr size_t size() noexcept { return sizeof...(_Vals); }
    };

    export template <class _Ty, _Ty _Size>
        using make_integer_sequence = __make_integer_seq<integer_sequence, _Ty, _Size>; //__make_integer_seq: compiler build-in function

    export template <size_t... _Vals>
        using index_sequence = integer_sequence<size_t, _Vals...>;

    export template <size_t _Size>
        using make_index_sequence = make_integer_sequence<size_t, _Size>;

    ///////////////////////////////////////////////////////////////
}

namespace mpl
{
	namespace traits
	{
		//>>>>>>>> remove_qualifier_t
		template<typename T>
		struct remove_qualifier_type
		{
			using type = remove_cvref_t<remove_pointer_t<T>>;
		};
		export template<typename T> using remove_qualifier_t = remove_qualifier_type<T>::type;
		//<<<<<<<<

		//>>>>>>>> is_component_name_v
		template<typename CompNameType>
		struct is_component_name_type
		{
			using raw_t = remove_qualifier_t<CompNameType>;
			enum { value = is_same_v<raw_t, char> || is_std_string_v<CompNameType> };
		};

		export template<typename T>
			inline constexpr bool is_component_name_v = is_component_name_type<T>::value;
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
			enum { value = is_same_v<First, Second> };
		};

		template<typename First, typename... Args>
		struct is_all_same_type<First, Args...>
		{
			using first_t = First;
			using next_t = is_all_same_type<Args...>::first_t;
			enum { value = is_same_v<first_t, next_t>&& is_all_same_type<Args...>::value };
		};

		export template<typename... Args>
			inline constexpr bool is_all_same_v = is_all_same_type<Args...>::value;
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

		export template<typename... Args>
			inline constexpr bool is_all_component_name_v = is_all_component_name_type<Args...>::value;
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
		template <typename T, typename = void_t<>>
		struct is_complete_t : false_type {};

		template <typename T>
		struct is_complete_t<T, void_t<decltype(sizeof(T))>> : true_type {};

		export template <typename T>
		inline constexpr bool is_complete_v = is_complete_t<T>::value;
		//<<<<<<<<

	}
}
