export module tecs:query;
//#include "configs_macro.h"

import std;
import typelist;
import configs;

import :core;
import :misc;
import :mpl;

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
            using pass_type = mpl::traits::select_if_else_t<has,
                                                            typename mpl::concat<mpl::typelist<Prev...>, arche_type>::type,
                                                            mpl::typelist<Prev...>>;

            using type = typename apply_t<N + 1, pass_type>::type;
        };
        
        template<typename... Prev>
        struct apply_t<configs::ARCHETYPE_TYPE_MAX_COUNT, mpl::typelist<Prev...>>
        {
            using type = mpl::typelist<Prev...>;
        };

        using type = typename apply_t<0, mpl::nulllist, void>::type;
    };

    template<typename... Args>
    struct query_t
    {
    private:
        using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<Args...>::type>::type;

        template<typename T, typename... TList>
        struct visit_t
        {
            //misc::type_id_archetype<T>::type;
        };
    };
}