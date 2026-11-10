export module tecs.advanced:query;

import std;
import typelist;
import configs;
import mpl;
import tecs.basic;
import tecs.datas;

import helper;

using size_t = std::size_t;

namespace tecs
{
    template<typename Queryable>
    concept QueryableConcept =     IsDerivedArchetypeConcept<Queryable>
                                || (IsDerivedBehaviourConcept<Queryable> && HasEachMethod<Queryable>)
                                || (!IsDerivedBehaviourConcept<Queryable> && std::is_standard_layout_v<Queryable>);

    template<typename T, typename ArgList> struct get_archetypes_of_component;
    template<typename T, typename ArgList>
    struct get_archetypes_of_component
    {
        template<size_t N, typename Prev, typename = void> struct apply_t { using type = Prev; };

        template<size_t N, typename Prev>
        struct apply_t<N, Prev, std::void_t<decltype(sizeof(archetype_of_id_t<N>))>>
        {
            enum { has = misc::type_map_t<T>:: template has<N>() };
            using arche_type = typename archetype_of_id_t<N>::type;
            using order_list = arche_type::order_list;
            using pass_type = mpl::traits::if_else_t<
                has,
                mpl::traits::if_else_t<mpl::is_sub_v<order_list, ArgList>,
                                       typename mpl::push_back_t<arche_type, Prev>,
                                       Prev>,
                Prev>;

            using type = typename apply_t<N + 1, pass_type>::type;
        };

        using type = typename apply_t<configs::TYPE_ID_START_INDEX, mpl::nulllist, void>::type;
    };
    export template<typename T, typename ArgList>
    using get_archetypes_of_component_t = get_archetypes_of_component<T, ArgList>::type;

    template<typename... Args>
    struct search_t
    {
    private:
        using unpack_list_t = mpl::no_duplicate_t<mpl::unpack_t<Args...>>;

        template<typename... TList> struct select_t;

        template<typename T, typename... TList>
        struct select_t<mpl::typelist<T, TList...>>
        {
            using type = typename get_archetypes_of_component_t<T, unpack_list_t>;
        };

        template<>
        struct select_t<mpl::nulllist>
        {
            using type = mpl::nulllist;
        };

    public:
        using type = typename select_t<unpack_list_t>::type;
    };

    template<QueryableConcept Arg, bool, bool>
    struct trait_component_type {  using type = Arg; };
    template<QueryableConcept Arg, bool IsBehaviour>
    struct trait_component_type<Arg, true, IsBehaviour>
    {  
        using type = Arg::order_list; 
    };
    template<QueryableConcept Arg, bool IsDerived>
    struct trait_component_type<Arg, IsDerived, true>
    {
        using type = mpl::nulllist;
    };
    template<QueryableConcept Arg, bool IsDerived, bool IsBehaviour>
    using trait_component_type_t = trait_component_type<Arg, IsDerived, IsBehaviour>::type;

    export struct query_t
    {
    private:
        template<typename Lambda, typename... As>
        static void run_impl(Lambda&& lambda, mpl::typelist<As...>)
        {
            (As::template each(std::forward<Lambda>(lambda)), ...);
        }
    
    public:
        template<typename Lambda>
        static void run(Lambda&& lambda)
        {
            using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
            using archetype_list = search_t<mpl::list_remove_qualifier_t<args_type>>::type;
            run_impl(std::forward<Lambda>(lambda), archetype_list{});
        }

        template<typename T> struct select_behaviour_t;
        template<typename T, typename... Behaviour>
        struct select_behaviour_t<mpl::typelist<T, Behaviour...>>
        {
            using behaviour = mpl::traits::if_else_t<HasEachMethod<T>, T, mpl::nulllist>;
            using type = mpl::concat<behaviour, typename select_behaviour_t<mpl::typelist<Behaviour...>>::type>::type;
        };
        template<>
        struct select_behaviour_t<mpl::nulllist> { using type = mpl::nulllist; };

        template<QueryableConcept... Queryable>
        struct with
        {
            using archetype_list = search_t<mpl::unpack_unpack_t<trait_component_type_t<Queryable, IsDerivedArchetypeConcept<Queryable>, IsDerivedBehaviourConcept<Queryable>>...>>::type;
            using behaviour_list = select_behaviour_t< mpl::typelist<Queryable...>>::type;
        private:

            template<typename TList, typename Arche> struct trait_behaviour_t;

            template<typename Arche, typename... Bs, template<typename> class BehaviourOrArche, typename... TList>
            struct trait_behaviour_t<mpl::typelist<BehaviourOrArche<Bs...>, TList...>, Arche>
            {
                using type = mpl::traits::if_else_t<mpl::has_v< mpl::typelist<Bs...>, Arche>,
                                                    BehaviourOrArche<Bs...>, 
                                                    typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type>;
                using default_type = typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::default_type;
            };
            template<typename Arche, typename T, typename... TList>
            struct trait_behaviour_t<mpl::typelist<T, TList...>, Arche>
            {
                using type = mpl::traits::if_else_t<std::is_base_of_v<Arche, T>, T, typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type>;
                using default_type = mpl::traits::if_else_t<std::is_base_of_v<Arche, T>, typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type, T>;
            };
            template<typename Arche> struct trait_behaviour_t<mpl::nulllist, Arche>
            {
                using type = mpl::nulllist;
                using default_type = mpl::nulllist;
            };

            template<typename Arche>
            static void loop_impl()
            {
                using found_type = trait_behaviour_t<behaviour_list, Arche>::type;
                using behaivour_type = mpl::traits::if_else_t<mpl::is_null_v<found_type>, typename trait_behaviour_t<behaviour_list, Arche>::default_type, found_type>;
                static_assert(!mpl::is_null_v<behaivour_type>, "you need to pass an empty specification class with static each() as a default caller: template<> struct something<> { each(...) }");
                if constexpr(!mpl::is_null_v<behaivour_type>)
                {
                    using args_type = utility::inspector_callable_t<decltype(&behaivour_type::each)>::args_type;
                    static_assert(mpl::is_sub_v<Arche::order_list, mpl::list_remove_qualifier_t<args_type>>, "args of each() are not belong to selections of `run`");
                    Arche::template each(&behaivour_type::each);
                }
            }

            template<typename Lambda, typename Arche>
            static void loop_impl(Lambda&& lambda, Arche)
            {
                using behaivour_type = trait_behaviour_t<behaviour_list, Arche>::type;
                if constexpr (mpl::is_null_v<behaivour_type>)
                {
                    using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
                    static_assert(mpl::is_sub_v<Arche::order_list, mpl::list_remove_qualifier_t<args_type>>, "args of lambda or function are not belong to selections of `run`");
                    Arche::template each(lambda);
                    return;
                }
                else
                {
                    using args_type = utility::inspector_callable_t<decltype(&behaivour_type::each)>::args_type;
                    static_assert(mpl::is_sub_v<Arche::order_list, mpl::list_remove_qualifier_t<args_type>>, "args of each() are not belong to selections of `run`");
                    Arche::template each(&behaivour_type::each);
                }
            }

            template<typename... As>
            static void loop(mpl::typelist<As...>)
            {
                (loop_impl<As>(), ...);
            }


            template<typename Lambda, typename... As>
            static void loop(Lambda&& lambda, mpl::typelist<As...>)
            {
                (loop_impl(std::forward<Lambda>(lambda), As{}), ...);
            }

        public:
            static void run()
            {
                loop(archetype_list{});
            }
            template<typename Lambda>
            static void run(Lambda&& lambda)
            {
                if constexpr(mpl::is_null_v<archetype_list>)
                { query_t::run(std::forward<Lambda>(lambda)); }
                else
                { loop(std::forward<Lambda>(lambda), archetype_list{}); }
            }
        };

        template<>
        struct with<>
        {
            template<typename Lambda>
            static void run(Lambda&& lambda)
            {
                query_t::run(std::forward<Lambda>(lambda));
            }
        };
    };

}