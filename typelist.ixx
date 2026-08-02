export module typelist;

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

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct push_front;

    template<typename T, typename... TList>
    struct push_front< T, typelist<TList...> >
    {
        typedef typelist<T, TList...>  type;
    };


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
    template<unsigned int N, typename... TList> struct at;

    template<unsigned int N, typename T, typename... TList>
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

    //////////////////////////////////////////////////////////
    template<int A, int B>
    struct IndexFixer
    {
        enum { value = (A == B) ? B : A + 1 };
    };

    //////////////////////////////////////////////////////////
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
    template<typename A, typename B> struct concat;

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



    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct erase;

    template<typename T, typename H, typename... TList>
    struct erase<T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename erase< T, typelist<TList...> >::type>::type type;
    };

    template<typename T, typename... TList>
    struct erase<T, typelist<T, TList...> >
    {
        typedef typelist<TList...> type;
    };

    template<typename T>
    struct erase<T, nulllist >
    {
        typedef nulllist type;
    };

    //////////////////////////////////////////////////////////
    template<typename T, typename... TList> struct erase_all;

    template<typename T, typename H, typename... TList>
    struct erase_all<T, typelist<H, TList...> >
    {
        typedef typename concat<H, typename erase_all< T, typelist<TList...> >::type>::type type;
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


    //////////////////////////////////////////////////////////
    template<typename T, typename...TList> struct no_duplicate;

    template<typename T, typename...TList>
    struct no_duplicate< typelist<T, TList...> >
    {
    private:
        typedef typename no_duplicate< typelist<TList...> >::type inner;
        typedef typename erase<T, inner>::type inner_result;
    public:
        typedef typename concat<T, inner_result>::type type;
    };

    template<>
    struct no_duplicate< nulllist >
    {
        typedef nulllist type;
    };


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

    //////////////////////////////////////////////////////////
    template<typename... TList> struct unpack;

    template<typename T, typename... TList>
    struct unpack<T, TList...>
    {
        using first = T;
        using rest = unpack<TList...>::type;
        using type = mpl::concat<first, rest>::type;
    };


    template<typename... Params, template<typename...> class T, typename... TList>
    struct unpack<T<Params...>, TList...>
    {
        using first = typelist<Params...>;
        using rest = unpack<TList...>::type;
        using type = mpl::concat<first, rest>::type;
    };

    template<>
    struct unpack<>
    {
        using type = mpl::nulllist;
    };
}