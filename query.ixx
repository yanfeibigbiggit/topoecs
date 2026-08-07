export module tecs.advanced:query;

import component_defines;
import archetype_defines;
import tecs.basic;

import std;
import typelist;
import configs;
import mpl;

import helper;

using size_t = std::size_t;

namespace tecs
{
    template<typename T>
    struct get_archetypes_of_component
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
    template<typename T>
    using get_archetypes_of_component_t = get_archetypes_of_component<T>::type;

    template<typename... Args>
    struct search_t
    {
    private:
        //using unpack_list_t = mpl::list_remove_qualifier_t<mpl::no_duplicate_t<mpl::push_front_t<tecs::entity_t, mpl::unpack_t<Args...>>>>;
        using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_t<Args...>>;

        template<typename... TList> struct select_t;

        template<typename T, typename... TList>
        struct select_t<mpl::typelist<T, TList...>>
        {
            using type = typename mpl::select_sublist_t<
                                    mpl::to_list_list_t<typename get_archetypes_of_component_t<T>>,
                                    unpack_list_t
                                                      >;
        };

        template<>
        struct select_t<mpl::nulllist>
        {
            using type = mpl::nulllist;
        };

    public:
        using type = typename select_t<unpack_list_t>::type;
    };

    template<typename... Ts> struct to_archetype_t;
    template<typename... Ts>
    struct to_archetype_t<mpl::typelist<Ts...>>
    {
        using type = archetype_storage_t<Ts...>;
        //you can't use archetype_t(which archetype_define_t<Args...>::type) here.
        //because misc::sort_index_sequence was call in a temlate function(in this case: query_t:each() function), and it cause unexpected ADL,
        //type_id_component will be no same as in component_defines and archetype_defines,
        //Anyway, we here depend on the first time type_id_component was build in archetype_defines, make things easy.
    };

    export struct query_t
    {
    private:
        template<typename Lambda, typename... As>
        static void each_impl(Lambda&& lambda, mpl::typelist<As...>)
        {
            //((std::cout<< typeid(to_archetype_t<As>::type).name() << std::endl), ...);
            ((typename to_archetype_t<As>::type::template each(lambda)), ...);
        }

    public:
        template<typename Lambda>
        static void each(Lambda&& lambda)
        {
            using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
            using archetype_list = search_t<mpl::list_remove_qualifier_t<args_type>>::type;
            each_impl(std::forward<Lambda>(lambda), archetype_list{});
        }

        /// <summary>
        /// archetypes of components you want to loop
        /// </summary>
        /// <typeparam name="...Args"></typeparam>
        template<typename... Args>
        struct with
        {
            using archetype_list = search_t<Args...>::type;

            template<typename Lambda>
            static void each(Lambda&& lambda)
            {
                using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
                
                static_assert(mpl::is_all_sublist_v<archetype_list, mpl::list_remove_qualifier_t<args_type>>, "args of lambda or function are not belong to selections of `with`");
                each_impl(std::forward<Lambda>(lambda), archetype_list{});
            }
        };
    };
}