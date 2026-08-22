export module typelist;

import mpl;
import std;

export namespace mpl
{
    //////////////////////////////////////////////////////////
    template<typename... TList>
    struct typelist
    {
    };

    typedef typelist<> nulllist;

    //////////////////////////////////////////////////////////
    template<typename... TList> struct length;

    template<typename... TList>
    struct length< typelist<TList...> >
    {
        enum { value = sizeof...(TList) };
    };
    template<typename TList> 
    constexpr inline size_t length_v = length<TList>::value;
    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct push_front;

    template<typename T, typename... TList>
    struct push_front< T, typelist<TList...> >
    {
        typedef typelist<T, TList...>  type;
    };
    template<typename T, typename TList> using push_front_t = push_front<T, TList>::type;

    //////////////////////////////////////////////////////////
    template<typename... TList> struct pop_front;

    template<typename T, typename... TList>
    struct pop_front< typelist<T, TList...> >
    {
        typedef typelist<TList...> type;
    };

    template<>
    struct pop_front< nulllist >
    {
        typedef nulllist type;
    };


    //////////////////////////////////////////////////////////
    template<size_t N, typename... TList> struct at;

    template<size_t N, typename T, typename... TList>
    struct at< N, typelist<T, TList...> >
    {
        typedef typename at< N - 1, typelist<TList...> >::type type;
    };

    template<typename T, typename... TList>
    struct at< 0, typelist<T, TList...> >
    {
        typedef T type;
    };

    template<>
    struct at< 0, nulllist >
    {
        typedef nulllist type;
    };
    template<size_t N, typename TList> using at_t = at<N, TList>::type;

    //////////////////////////////////////////////////////////
    template<int A, int B>
    struct IndexFixer
    {
        enum { value = (A == B) ? B : A + 1 };
    };

    template<typename T, typename... TList> struct indexof;

    template<typename T, typename H, typename... TList>
    struct indexof< T, typelist<H, TList...> >
    {
        enum { value = IndexFixer<indexof<T, typelist<TList...>>::value, -1>::value };
    };

    template<typename T, typename... TList>
    struct indexof< T, typelist<T, TList...> >
    {
        enum { value = 0 };
    };

    template<typename T>
    struct indexof< T, nulllist >
    {
        enum { value = -1 };
    };

    //////////////////////////////////////////////////////////
    template<typename A, typename B> struct concat { typedef typelist<A, B> type; };

    template<typename... A, typename... B>
    struct concat<typelist<A...>, typelist<B...> >
    {
        typedef typelist<A..., B...> type;
    };

    template<typename T, typename... TList>
    struct concat<typelist<TList...>, T >
    {
        typedef typelist<TList..., T> type;
    };

    template<typename T, typename... TList>
    struct concat< T, typelist<TList...> >
    {
        typedef typelist<T, TList...>  type;
    };

    template<typename A, typename B> using concat_t = concat<A, B>::type;

    //////////////////////////////////////////////////////////
    //concat L and R. if R is list, unpack it. And never unpack L.
    template<typename L, typename R> struct concat_right
    {
        using type = typelist<L, R>;
    };

    template<typename L, typename... R>
    struct concat_right<L, typelist<R...>>
    {
        using type = typelist<L, R...>;
    };


    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct erase;

    template<typename T, typename H, typename... TList>
    struct erase<T, typelist<H, TList...> >
    {
        typedef typename concat_right<H, typename erase< T, typelist<TList...> >::type>::type type;
    };

    template<typename T, typename... TList>
    struct erase<T, typelist<T, TList...> >
    {
        typedef erase<T, typelist< TList...>>::type type;
    };

    template<typename T>
    struct erase<T, nulllist >
    {
        typedef nulllist type;
    }; 

    template<typename T, typename TList> using erase_t = erase<T, TList>::type;

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct erase_all;
    template<typename T, typename H, typename... TList>
    struct erase_all<T, typelist<H, TList...> >
    {
        typedef typename concat_right<H, typename erase_all< T, typelist<TList...> >::type>::type type;
    };

    template<typename T, typename... TList>
    struct erase_all<T, typelist<T, TList...> >
    {
        typedef typename erase_all< T, typelist<TList...> >::type type;
    };

    template<typename T>
    struct erase_all<T, nulllist >
    {
        typedef nulllist type;
    };
    template<typename T, typename TList> using erase_all_t = erase_all<T, TList>::type;

    //////////////////////////////////////////////////////////
    template<typename T, typename...TList> struct no_duplicate;

    template<typename T, typename...TList>
    struct no_duplicate< typelist<T, TList...> >
    {
    private:
        typedef typename no_duplicate< typelist<TList...> >::type inner;
        typedef typename erase<T, inner>::type inner_result;
    public:
        typedef typename concat_right<T, inner_result>::type type;
    };

    template<>
    struct no_duplicate< nulllist >
    {
        typedef nulllist type;
    };
    template<typename... TList> using no_duplicate_t = no_duplicate<TList...>::type;

    //////////////////////////////////////////////////////////
    template<typename R, typename T, typename...TList> struct replace;

    template<typename R, typename T, typename H, typename...TList>
    struct replace<R, T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename replace<R, T, typelist<TList...>>::type>::type type;
    };

    template<typename R, typename H, typename...TList>
    struct replace<R, H, typelist<H, TList...> >
    {
        typedef typename concat<R, typelist<TList...> >::type type;
    };

    template<typename R, typename T>
    struct replace<R, T, nulllist >
    {
        typedef nulllist type;
    };

    //////////////////////////////////////////////////////////
    template<typename R, typename T, typename...TList> struct replace_all;

    template<typename R, typename T, typename H, typename...TList>
    struct replace_all<R, T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename replace_all<R, T, typelist<TList...>>::type>::type type;
    };

    template<typename R, typename H, typename...TList>
    struct replace_all<R, H, typelist<H, TList...> >
    {
        typedef typename concat<R, typename replace_all<R, H, typelist<TList...>>::type >::type type;
    };

    template<typename R, typename T>
    struct replace_all<R, T, nulllist >
    {
        typedef nulllist type;
    };

    //////////////////////////////////////////////////////////
    template<typename T, typename...TList> struct reverse;

    template<typename T, typename...TList>
    struct reverse<typelist<T, TList...> >
    {
        typedef typename concat<typename reverse<typelist<TList...>>::type, T>::type type;
    };

    template<>
    struct reverse< nulllist >
    {
        typedef nulllist type;
    };
    template<typename...TList> using reverse_t = reverse<TList...>::type;

    //////////////////////////////////////////////////////////
    template<typename T> struct is_null { enum { value = 0 }; };
    template<> 
    struct is_null<nulllist> { enum { value = 1 }; };

    template<typename T>
    constexpr inline bool is_null_v = is_null<T>::value;

    //////////////////////////////////////////////////////////
    template<typename T> struct is_list { enum { value = is_null_v<T> }; };
    template<typename... Ts>
    struct is_list<typelist<Ts...>> { enum { value = 1 }; };

    template<typename T>
    constexpr inline bool is_list_v = is_list<T>::value;

    //////////////////////////////////////////////////////////
    //typelist<host<...>, a, b, ...> => typelist<typelist<...>, a, b, ...>
    template<typename... TList> struct to_list_if_template;

    template<typename T>
    struct to_list_if_template<T>
    {
        using type = T;
    };

    template<template<typename> class Host, typename... TList>
    struct to_list_if_template<Host<TList...>>
    {
        using type = typelist<TList...>;
    };

    template<>
    struct to_list_if_template<nulllist>
    {
        using type = nulllist;
    };

    template<typename TList>
    using to_list_if_template_t = to_list_if_template<TList>::type;

    //////////////////////////////////////////////////////////
    template<typename... TList> struct unpack;

    template<typename T, typename... TList>
    struct unpack<T, TList...>
    {
        using type = concat<T, typename unpack<TList...>::type>::type;
    };

    template<typename... Params, template<typename...> class T, typename... TList>
    struct unpack<T<Params...>, TList...>
    {
        using type = concat<typelist<Params...>, typename unpack<TList...>::type>::type;
    };

    template<>
    struct unpack<>
    {
        using type = nulllist;
    };
    template<typename... TList> using unpack_t = unpack<TList...>::type;

    //////////////////////////////////////////////////////////
    template<typename... TList> struct unpack_unpack;

    template<typename T, typename... TList>
    struct unpack_unpack<T, TList...>
    {
        using type = concat<T, typename unpack_unpack<TList...>::type>::type;
    };

    template<typename... Params, template<typename...> class T, typename... TList>
    struct unpack_unpack<T<Params...>, TList...>
    {
        using type = concat<typename unpack_unpack<Params...>::type, typename unpack_unpack<TList...>::type>::type;
    };

    template<>
    struct unpack_unpack<>
    {
        using type = nulllist;
    };
    template<typename... TList> using unpack_unpack_t = unpack_unpack<TList...>::type;

    //////////////////////////////////////////////////////////
    //other<a,b,...> => typelist<a, b,...>
    template<typename... TList> struct to_list;
    template<template<typename...> class Pack, typename... TList>
    struct to_list < Pack<TList...> >
    {
        using type = typelist<TList...>;
    };
    template<typename... TList> using to_list_t = to_list<TList...>::type;

    //////////////////////////////////////////////////////////
    //typlist<other<...>, ...>  => typelist<typelist<...>, ...>
    template<typename... TList> struct to_list_list;
    template<typename... TList>
    struct to_list_list< typelist<TList...>>
    {
        using type = typelist<typename to_list<TList>::type...>;
    };
    template<> 
    struct to_list_list<nulllist> { using type = nulllist; };
    template<typename... TList>
    using to_list_list_t = to_list_list<TList...>::type;
    //////////////////////////////////////////////////////////
    template<typename TList, typename T> struct has;

    template<typename T, typename...TList>
    struct has<typelist< TList...>, T>
    {
        enum { value = (std::is_same_v<TList, T> || ... ) };
    };
    template<typename TList, typename T>
    constexpr inline bool has_v = has<TList, T>::value;

    //////////////////////////////////////////////////////////
    template<typename Main, typename Sub> struct is_sub;
    //return true if sub is a sub set of main.
    template<typename... Main, typename... Sub>
    struct is_sub<typelist<Main...>, typelist<Sub...>>
    {
        enum { value = (has_v<typelist<Main...>, Sub> && ...) };
    };

    template<typename Main, typename Sub>
    constexpr inline bool is_sub_v = is_sub<Main, Sub>::value;

    //////////////////////////////////////////////////////////
    //all list of Main contain sub
    template<typename Main, typename Sub> struct is_all_sublist;
    template<typename T, typename... Main, typename Sub>
    struct is_all_sublist<typelist<T, Main...>, Sub>
    {
        enum { value = is_sub_v<T, Sub >&& is_all_sublist <typelist< Main...>, Sub > ::value };
    };

    template<typename Sub>
    struct is_all_sublist<typelist<>, Sub>
    {
        enum { value = 1 };
    };

    template<typename Main, typename Sub>
    constexpr inline bool is_all_sublist_v = is_all_sublist<Main, Sub>::value;

    //////////////////////////////////////////////////////////
    //return list of list that contain all element of sub list
    template<typename Main, typename Sub> struct listlist_has_sublist;

    template<typename... Main, typename... Sub>
    struct listlist_has_sublist<typelist<Main...>, typelist<Sub...>>
    {
        using temp = typelist<traits::if_else_t<is_sub_v<Main, typelist<Sub...>>, Main, nulllist>...>;
        using type = typename erase_all<nulllist, temp>::type;
    };
    template<typename Main, typename Sub> using listlist_has_sublist_t = listlist_has_sublist<Main, Sub>::type;

    //////////////////////////////////////////////////////////
    //return first list that contain Target
    template<typename Main, typename Target> struct firstlist_has;

    template<typename T, typename... Main, typename Target>
    struct firstlist_has<typelist<T, Main...>, Target>
    {
        using type = traits::if_else_t<has_v<T, Target>, T, typename firstlist_has<typelist<Main...>, Target>::type>;
    };
    template<typename Target> 
    struct firstlist_has<nulllist, Target>
    {
        using type = nulllist;
    };
    template<typename Main, typename Target> using firstlist_has_t = firstlist_has<Main, Target>::type;

    //////////////////////////////////////////////////////////
    //return all list belong to MainList and contain any type in SubList
    template<typename Main, typename Sub> struct listlist_has_any_of;
    template<typename Main, typename... Sub> 
    struct listlist_has_any_of<Main, typelist<Sub...>>
    {
        template<typename TList, typename Prev> struct In;
        template<typename T, typename... TList, typename Prev>
        struct In<typelist<T, TList...>, Prev>
        {
            using type = In<typelist<TList...>, traits::if_else_t<std::is_same_v<at_t<1, T>, nulllist>, Prev, push_front_t<T, Prev>>>::type;
        };
        template<typename Prev>
        struct In<nulllist, Prev>
        {
            using type = Prev;
        };

        using type = typename In<typelist<typelist<Sub, firstlist_has_t<Main, Sub>>...>, nulllist>::type;
    };
    template<typename Main, typename Sub> using listlist_has_any_of_t = listlist_has_any_of<Main, Sub>::type;

    //////////////////////////////////////////////////////////
    template<typename TMap, typename Tkey> struct pairs_find;
    template<typename TPair, typename... TMap, typename Tkey> 
    struct pairs_find<typelist<TPair, TMap...>, Tkey>
    {
        using type = traits::if_else_t<std::is_same_v<at_t<0, TPair>, Tkey>, at_t<1, TPair>, typename pairs_find<typelist<TMap...>, Tkey>::type>;
    };
    template<typename Tkey> 
    struct pairs_find<nulllist, Tkey>
    {
        using type = nulllist;
    };
    template<typename TMap, typename Tkey> using pairs_find_t = pairs_find<TMap, Tkey>::type;

    //////////////////////////////////////////////////////////
    template<typename TMap> struct pairs_values;
    template<typename... TMap> 
    struct pairs_values<typelist<TMap...>>
    {
        using type = typelist<at_t<1, TMap>...>;
    };
    template<typename TMap> using pairs_values_t = pairs_values<TMap>::type;
    
    //////////////////////////////////////////////////////////
    template<typename... TList> struct list_remove_qualifier;

    template<typename... TList> 
    struct list_remove_qualifier<typelist<TList...>>
    {
        using type = typelist<traits::remove_qualifier_t<TList>...>;
    };
    template<typename... TList>
    using list_remove_qualifier_t = list_remove_qualifier<TList...>::type;

    //////////////////////////////////////////////////////////
    template<typename TList, typename T> struct first_of_not;

    template<typename Target, typename T, typename... TList>
    struct first_of_not<typelist<T, TList...>, Target>
    {
        using type = traits::if_else_t<std::is_same_v<T, Target>, typename first_of_not<typelist<TList...>, Target>::type, T>;
    };
    template<typename T> 
    struct first_of_not<typelist<>, T>
    {
        using type = nulllist;
    };
    template<typename TList, typename T> using first_of_not_t = first_of_not<TList, T>::type;

    //////////////////////////////////////////////////////////
    //has raw reference of Target. raw type of( Target&, Target*,....) is Target.
    template<typename TList, typename Target> struct has_raw_reference_of_t;

    template<typename Target, typename... TList> 
    struct has_raw_reference_of_t<typelist<TList...>, Target>
    {
        using raw_taregt = traits::remove_qualifier_t<Target>;
        enum { value = ((std::is_same_v<traits::remove_qualifier_t<TList>, raw_taregt> && std::is_reference_v<TList>) || ...) };
    };

    template<typename TList, typename Target> constexpr inline bool has_raw_reference_of_v = has_raw_reference_of_t<TList, Target>::value;

    //////////////////////////////////////////////////////////
    //has any of that raw type is Target, ignore any qualifier. raw type of( Target&, Target*,....) is Target.
    template<typename TList, typename Target> struct has_raw_of_t;

    template<typename Target, typename... TList>
    struct has_raw_of_t<typelist<TList...>, Target>
    {
        using raw_taregt = traits::remove_qualifier_t<Target>;
        enum { value = (std::is_same_v<traits::remove_qualifier_t<TList>, raw_taregt> || ...) };
    };

    template<typename TList, typename Target> constexpr inline bool has_raw_of_v = has_raw_of_t<TList, Target>::value;

    //////////////////////////////////////////////////////////
    // typelist<a,b...> has Target, or T is Target
    template<typename TList, typename T> struct same_or_has { enum { value = 0 }; };

    template<typename T, typename... TList, typename Target>
    struct same_or_has<typelist<T, TList...>, Target>
    {
        enum { value = std::is_same_v<T, Target> || same_or_has<TList...>::value };
    };

    template<typename... Params, template<typename> class H, typename... TList, typename Target>
    struct same_or_has<typelist<H<Params...>, TList...>, Target>
    {
        enum { value = has_v<typelist<Params...>, Target> || same_or_has<TList...>::value };
    };
    template<typename T>
    struct same_or_has<T, T> { enum { value = 1 }; };

    template<typename... Params, template<typename> class H, typename Target>
    struct same_or_has<H<Params...>, Target>
    {
        enum { value = has_v<typelist<Params...>, Target> };
    };

    template<typename Target>
    struct same_or_has<nulllist, Target> { enum {value = 0}; };

    template<typename TList, typename Target> constexpr inline bool same_or_has_v = same_or_has<TList, Target>::value;

    //////////////////////////////////////////////////////////
    template<typename TList, typename T> struct first_of_has;

    template<typename Target, typename T, typename... TList>
    struct first_of_has<typelist<T, TList...>, Target>
    {
        using first = T;
        using type = traits::if_else_t<same_or_has_v<T, Target>, T, typename first_of_has<typelist<TList...>, Target>::type>;
    };
    template<typename T>
    struct first_of_has<typelist<>, T>
    {
        using type = nulllist;
    };
    template<typename TList, typename T> using first_of_has_t = first_of_has<TList, T>::type;

    //////////////////////////////////////////////////////////
    template<typename TList, typename Removes> struct remove_any_of_type;
    template<typename TList, typename R, typename... Removes>
    struct remove_any_of_type<TList, typelist<R, Removes...>>
    {
        using type = erase_all_t<erase_all_t<R, TList>, typelist<Removes...>>;
    };
    template<typename TList> 
    struct remove_any_of_type<TList, nulllist> { using type = TList; };
    template<typename Removes>
    struct remove_any_of_type<nulllist, Removes>  {  using type = nulllist; };

    template<typename TList, typename Removes> using remove_any_of_t = remove_any_of_type<TList, Removes>::type;

    

}

export namespace mpl
{
    namespace seq
    {
        //////////////////////////////////////////////////////////
        template<size_t... Is>
        struct indexlist {};

        typedef indexlist<> nulllist;

        //////////////////////////////////////////////////////////
        template<size_t... Is> struct length { enum { value = sizeof...(Is) }; };

        //////////////////////////////////////////////////////////
        template<size_t I, typename Is> struct push_front;

        template<size_t I, size_t... Is>
        struct push_front< I, indexlist<Is...> >
        {
            using type = indexlist<I, Is...>;
        };
        template<size_t I, typename Is> using push_front_t = push_front<I, Is>::type;

        //////////////////////////////////////////////////////////
        template<typename Is> struct pop_front;

        template<size_t I, size_t... Is>
        struct pop_front<indexlist<I, Is...> >
        {
            using type = indexlist<Is...>;
        };

        template<>
        struct pop_front<nulllist>
        {
            using type = nulllist;
        };
        template<typename Is> using pop_front_t = pop_front<Is>::type;

        //////////////////////////////////////////////////////////
        template<size_t N, typename Is> struct at;

        template<size_t N, size_t I, size_t... Is>
        struct at< N, indexlist<I, Is...> >
        {
            static constexpr inline size_t value = at< N - 1, indexlist<Is...> >::value;
        };

        template<size_t I, size_t... Is>
        struct at< 0, indexlist<I, Is...> >
        {
            static constexpr inline size_t value = I;
        };

        template<>
        struct at< 0, nulllist >
        {
            static constexpr inline size_t value = -1;
        };
        template<size_t N, typename TList> using at_t = at<N, TList>::value;


        //////////////////////////////////////////////////////////
        template<typename A, typename B> struct concat;

        template<size_t... A, size_t... B>
        struct concat<indexlist<A...>, indexlist<B...> >
        {
            using type = indexlist<A..., B...>;
        };

        template<typename A, typename B> using concat_t = concat<A, B>::type;

        //////////////////////////////////////////////////////////
        template<typename Is> struct max;

        template<size_t I, size_t... Is>
        struct max<indexlist<I, Is...>>
        {
            static constexpr inline size_t value = I > indexlist<Is...>::value ? I : indexlist<Is...>::value;
        };
        template<>
        struct max<nulllist>
        {
            static constexpr inline size_t value = -1;
        };
    }
}
