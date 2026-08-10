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
    template<typename Arche> struct is_archetype_t { static constexpr bool value = 0; };
    template<typename... Ts> struct is_archetype_t<archetype_storage_t<Ts...>> { static constexpr bool value = 1; };
    template<typename Arche> concept is_archetype_c = is_archetype_t<Arche>::value;

    template<typename Behaviour> struct is_behaviour_template_t { static constexpr bool value = 0; };
    template<is_archetype_c... As, template<typename> class Behaviour>
    struct is_behaviour_template_t <Behaviour<As...>> { static constexpr bool value = 1; };
    template<typename Behaviour> concept is_behaviour_template_c = is_behaviour_template_t<Behaviour>::value;

    export template<typename T, typename... Args> 
    concept has_static_each_c = requires(Args... args) 
    {
        { T::each(args...) } -> std::same_as<void>;
    };
    //another implementation
    //template <class C>
    //concept HasEachMethod = std::is_function_v<decltype(C::each)>;
    export template <class C>
    concept HasEachMethod = std::is_function_v<decltype(C::each)>;


    template<typename Behaviour>
    concept is_arg_of_query_run_c = is_archetype_c<Behaviour> || (is_behaviour_template_c<Behaviour> && has_static_each_c<Behaviour>);

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
        using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_t<Args...>>;

        template<typename... TList> struct select_t;

        template<typename T, typename... TList>
        struct select_t<mpl::typelist<T, TList...>>
        {
            using type = typename mpl::select_sublist_t<
                                    mpl::to_list_list_t<typename get_archetypes_of_component_t<T>>,
                                    unpack_list_t>;
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
    };

    export struct query_t
    {
    private:
        template<typename Lambda, typename... As>
        static void each_impl(Lambda&& lambda, mpl::typelist<As...>)
        {
            //((std::cout<< typeid(to_archetype_t<As>::type).name() << std::endl), ...);
            (typename to_archetype_t<As>::type::template each(lambda), ...);
        }

    public:
        template<typename Lambda>
        static void each(Lambda&& lambda)
        {
            using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
            using archetype_list = search_t<mpl::list_remove_qualifier_t<args_type>>::type;
            each_impl(std::forward<Lambda>(lambda), archetype_list{});
        }

        template<typename... Args>
        struct with_t
        {
            using archetype_list = search_t<Args...>::type;

            template<typename Lambda>
            static void operator()(Lambda&& lambda)
            {
                using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
                static_assert(mpl::is_all_sublist_v<archetype_list, mpl::list_remove_qualifier_t<args_type>>, "args of lambda or function are not belong to selections of `with`");
                each_impl(std::forward<Lambda>(lambda), archetype_list{});
            }
        };

        template<typename... Behaviour>
        requires (is_arg_of_query_run_c<Behaviour> && ...)
        struct run_t
        {
            using archetype_list = search_t<mpl::unpack_unpack_t<Behaviour...>>::type;
            using hierarchy = mpl::typelist< Behaviour...>;
        private:

            template<typename TList, typename Arche> struct trait_behaviour_t
            {
                using type = mpl::at_t<0, TList>;
            };

            template<typename Arche, typename... Bs, template<typename> class Behaviour, typename... TList>
            struct trait_behaviour_t<mpl::typelist<Behaviour<Bs...>, TList...>, Arche>
            {
                using type = mpl::traits::if_else_t<mpl::is_same_v<mpl::to_list_t<mpl::at_t<0, mpl::typelist<Bs...>>>, Arche>, 
                                                    Behaviour<Bs...>, 
                                                    typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type>;
            };

            template<typename Arche>
            static void loop_impl()
            {
                using behaivour_type = trait_behaviour_t<hierarchy, Arche>::type;
                static_assert(!mpl::is_null_v<behaivour_type>, "you need to offer a default each() to run. like: template<> class something<> { each(...) {...}");
                if constexpr(mpl::is_null_v<behaivour_type>)//prevent compiler error unneed.
                { return; }
                else
                {
                    using args_type = utility::inspector_callable_t<decltype(&behaivour_type::each)>::args_type;
                    static_assert(mpl::is_sub_v<mpl::to_list_t<typename to_archetype_t<Arche>::type>, mpl::list_remove_qualifier_t<args_type>>, "args of lambda or function are not belong to selections of `run`");
                    typename to_archetype_t<Arche>::type::template each(&behaivour_type::each);
                }
            }

            template<typename... As>
            static void loop(mpl::typelist<As...>)
            {
                (loop_impl<As>(), ...);
            }

        public:
            static void operator()()
            {
                loop(archetype_list{});
            }
        };


        template<typename... Arches>
        static constexpr inline with_t<Arches...> with = with_t<Arches...>{};
        template<typename... Behaviour>
            //requires (is_arg_of_query_run_c<Behaviour> && ...)
        static constexpr inline run_t<Behaviour...> run = run_t<Behaviour...>{};
    };


    //export template<typename... Args>
    struct hierarchy_task_t
    {
        static void run()
        {

        }
    };
}