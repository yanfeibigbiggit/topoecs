export module type_linear_map;

import std;
import typelist;

namespace mpl
{
    template<typename first_, typename second_>
    struct pair
    {
        typedef first_ first;
        typedef second_ second;
    };

    //linear map
    //////////////////////////////////////////////////////////
    template<typename... TList> struct build_linear_map;

    template<typename... TList>
    struct build_linear_map
    {
        typedef typelist<TList...> type;
    };

    template<typename... TList>
    struct build_linear_map< typelist<TList...> >
    {
        typedef typelist<TList...> type;
    };

    template<typename A, typename... AList, typename B, typename... BList>
    struct build_linear_map< typelist<A, AList...>, typelist<B, BList...> >
    {
        static_assert(sizeof...(AList) == sizeof...(BList), "length must be same");
        typedef typename concat< pair<A, B>,
            typename build_linear_map< typelist<AList...>, typelist<BList...> >::type >::type type;
    };

    template<typename A, typename B>
    struct build_linear_map< typelist<A>, typelist<B> >
    {
        typedef typelist< pair<A, B> > type;
    };

    template<>
    struct build_linear_map< typelist<>, typelist<> >
    {
        typedef typelist<> type;
    };

    //////////////////////////////////////////////////////////
    template<typename Key, typename TList> struct linear_map_find;

    template<typename Key, typename T, typename... TList>
    struct linear_map_find<Key, typelist<T, TList...>>
    {
        typedef typename linear_map_find<Key, typelist<TList...> >::type type;
    };

    template<typename Key, typename Second, typename... TList>
    struct linear_map_find<Key, typelist< pair<Key, Second>, TList... >>
    {
        typedef Second type;
    };

    template<typename Key>
    struct linear_map_find<Key, nulllist>
    {
        typedef nulllist type;
    };

    //////////////////////////////////////////////////////////
    template<typename Key, typename TList> struct linear_map_find_all;

    template<typename Key, typename T, typename... TList>
    struct linear_map_find_all<Key, typelist<T, TList...>>
    {
        typedef typename linear_map_find_all<Key, typelist<TList...> >::type type;
    };

    template<typename Key, typename Second, typename... TList>
    struct linear_map_find_all<Key, typelist< pair<Key, Second>, TList... > >
    {
        typedef typename concat<Second, typename linear_map_find_all<Key, typelist<TList...> >::type >::type type;
    };

    template<typename Key>
    struct linear_map_find_all<Key, nulllist>
    {
        typedef nulllist type;
    };

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_add;

    template<typename T, typename... TList>
    struct linear_map_add< T, typelist<TList...> >
    {
        typedef typename concat<T, typelist<TList...> >::type type;
    };

    template<typename Key, typename NewValue, typename... TList>
    struct linear_map_add<Key, NewValue, typelist<TList...> >
    {
        typedef typename concat<pair<Key, NewValue>, typelist<TList...> >::type type;
    };

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_insert;

    template<typename T, typename... TList>
    struct linear_map_insert<T, typelist<TList...> >
    {
        static_assert(std::is_same<
            typename linear_map_find<typename T::first, typelist<TList...> >::type,
            nulllist>::value, "key already exist");
        typedef typename concat<T, typelist<TList...> >::type type;
    };

    template<typename First, typename Second, typename... TList>
    struct linear_map_insert<First, Second, typelist<TList...> >
    {
        static_assert(std::is_same<
            typename linear_map_find<First, typelist<TList...> >::type,
            nulllist>::value, "key already exist");
        typedef typename concat<pair<First, Second>, typelist<TList...> >::type type;
    };


    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_replace;

    template<typename T, typename H, typename... TList>
    struct linear_map_replace<T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename linear_map_replace<T, typelist<TList...> >::type>::type type;
    };

    template<typename Key, typename NewValue, typename OldValue, typename... TList>
    struct linear_map_replace<pair<Key, NewValue>, typelist<pair<Key, OldValue>, TList...> >
    {
        typedef typename concat<pair<Key, NewValue>, typelist<TList...> >::type type;
    };

    template<typename T>
    struct linear_map_replace<T, nulllist>
    {
        typedef nulllist type;
    };

    template<typename Key, typename NewValue, typename... TList>
    struct linear_map_replace<Key, NewValue, typelist<TList...> >
    {
        typedef typename linear_map_replace<pair<Key, NewValue>, typelist<TList...> >::type type;
    };

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_earse;

    template<typename T, typename H, typename... TList>
    struct linear_map_earse<T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename linear_map_earse<T, typelist<TList...> >::type>::type type;
    };

    template<typename Key, typename Value_, typename... TList>
    struct linear_map_earse<Key, typelist<pair<Key, Value_>, TList...> >
    {
        typedef typelist<TList...> type;
    };

    template<typename T>
    struct linear_map_earse<T, nulllist>
    {
        typedef nulllist type;
    };


    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_earse_all;

    template<typename T, typename H, typename... TList>
    struct linear_map_earse_all<T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename linear_map_earse_all<T, typelist<TList...> >::type>::type type;
    };

    template<typename Key, typename Value_, typename... TList>
    struct linear_map_earse_all<Key, typelist<pair<Key, Value_>, TList...> >
    {
        typedef typename linear_map_earse_all<Key, typelist<TList...>>::type type;
    };

    template<typename T>
    struct linear_map_earse_all<T, nulllist>
    {
        typedef nulllist type;
    };


    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct linear_map_no_duplicate;

    template<typename T, typename...TList>
    struct linear_map_no_duplicate< typelist<T, TList...> >
    {
    private:
        typedef typename linear_map_no_duplicate< typelist<TList...> >::type inner;
        typedef typename linear_map_earse<typename T::first, inner>::type inner_result;
    public:
        typedef typename concat<T, inner_result>::type type;
    };

    template<>
    struct linear_map_no_duplicate< nulllist >
    {
        typedef nulllist type;
    };
}


