export module tecs:query;
//#include "configs_macro.h"

import std;
import typelist;
import configs;
import mpl;

import :core;
import :misc;


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
        
        template<typename... Prev>
        struct apply_t<configs::ARCHETYPE_TYPE_MAX_COUNT, mpl::typelist<Prev...>>
        {
            using type = mpl::typelist<Prev...>;
        };

        using type = typename apply_t<0, mpl::nulllist, void>::type;
    };

    export template<typename... Args>
    struct query_t
    {
    private:
        using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<Args...>::type>::type;

        template<typename... TList> struct select_t;
        //template<typename T, typename... TList>
        //struct select_t<mpl::typelist<T, TList...>>
        //{
        //    //using temp1 = typename get_archetypes_of_component_t<T>::type;
        //    //using temp2 = mpl::to_list_list<temp1>::type;
        //    //using temp3 = typename mpl::list_has_sub<temp2, unpack_list_t>::type;
        //    //using temp4 = typename mpl::concat_right< temp3, typename select_t<mpl::typelist<TList...>>::type>::type;
        //    //using temp5 = mpl::no_duplicate<temp4>::type;
        //    //using type = temp5;

        //    using temp3 = typename mpl::list_has_sub<typename mpl::to_list_list<typename get_archetypes_of_component_t<T>::type>::type, unpack_list_t>::type;
        //    using temp4 = typename mpl::concat_right< temp3, typename select_t<mpl::typelist<TList...>>::type>::type;
        //    using temp5 = mpl::no_duplicate<temp4>::type;
        //    using type = temp5;
        //};

        template<typename... TList>
        struct select_t<mpl::typelist<TList...>>
        {
            using temp1 = mpl::typelist<
                            typename mpl::unpack< 
                                typename mpl::list_has_sub<
                                    typename mpl::to_list_list<typename get_archetypes_of_component_t<TList>::type>::type, 
                                    unpack_list_t
                                                          >::type
                                                >::type...
                          >;
            using temp2 = mpl::no_duplicate<temp1>::type;
            using type = mpl::erase_all<mpl::nulllist, temp2>::type;
        };

        template<>
        struct select_t<mpl::nulllist>
        {
            using type = mpl::nulllist;
        };
    public:
        using archetype_list = typename select_t<unpack_list_t>::type;
    };


}