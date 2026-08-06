export module tecs.advanced:query;

import tecs.basic;
import component_defines;
import archetype_defines;

import std;
import typelist;
import configs;
import mpl;



using size_t = std::size_t;

namespace tecs
{
    export template<typename T>
    struct get_archetypes_of_component_t
    {
        template<size_t N, typename Prev, typename = void> struct apply_t { using type = Prev; };

        template<size_t N, typename... Prev>
        struct apply_t<N, mpl::typelist<Prev...>, mpl::void_t<decltype(sizeof(archetype_of_id_t<N>))>>
        {
            enum { has = misc::type_map_t<T>:: template has<N>() };
            using arche_type = typename archetype_of_id_t<N>::type;
            using pass_type = mpl::traits::if_else_t<has,
                                                            typename mpl::concat<mpl::typelist<Prev...>, arche_type>::type,
                                                            mpl::typelist<Prev...>>;

            using type = typename apply_t<N + 1, pass_type>::type;
        };

        using type = typename apply_t<configs::TYPE_ID_START_INDEX, mpl::nulllist, void>::type;
    };

    export template<typename... Args>
    struct query_t
    {
    private:
        using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<Args...>::type>::type;

        template<typename... TList> struct select_t;

        template<typename T, typename... TList>
        struct select_t<mpl::typelist<T, TList...>>
        {
            using type = typename mpl::list_has_sub<
                typename mpl::to_list_list<typename get_archetypes_of_component_t<T>::type>::type,
                unpack_list_t
                                                   >::type;
        };

        template<>
        struct select_t<mpl::nulllist>
        {
            using type = mpl::nulllist;
        };

    public:
        using archetype_list = typename select_t<unpack_list_t>::type;

        template<typename... Ts> struct to_archetype_t;
        template<typename... Ts>
        struct to_archetype_t<mpl::typelist<Ts...>>
        {
            using type = archetype_t<Ts...>;
        };
    
    private:
        template<typename Lambda, typename... As>
        static void each_impl(Lambda&& lambda, mpl::typelist<As...>)
        {
            ((typename to_archetype_t<As>::type::template each(lambda)), ...);
        }

    public:
        template<typename Lambda>
        static void each(Lambda&& lambda)
        {
            each_impl(std::forward<Lambda>(lambda), archetype_list{});
        }
    };


}