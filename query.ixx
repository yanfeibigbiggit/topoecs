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
    export template<typename Arche> 
    concept IsArchetypeConcept = is_archetype_t<Arche>::value;

    export template<class C> concept IsTemplateClassConcept = requires(C c)
    {
        [] <template<class...> class T, class... Ts>(T<Ts...>) {}(c);
    };

    template<typename Behaviour> struct is_query_behaviour_t { static constexpr bool value = 0; };
    template<IsArchetypeConcept... As, template<typename> class Behaviour>
    struct is_query_behaviour_t<Behaviour<As...>> { static constexpr bool value = 1; };
    template<typename Behaviour>
    concept IsQueryBehaviourConcept = is_query_behaviour_t<Behaviour>::value;

    //-- don't work --//
    //export template<typename T, typename... Args> 
    //concept HasStaticEachFuncConcept = requires(Args... args)
    //{
    //    { T::each(args...) } -> std::same_as<void>;
    //};
    //another implementation
    export template <class C>
    concept HasEachMethod = std::is_function_v<decltype(C::each)>;

    template <class C>
    concept IsDerivedFromArchetypeConcept = requires(C c) 
    {
        // IILE, that only binds to A<...> specialisations
        // Including classes derived from them
        [] <typename... X>(archetype_storage_t<X...>&) {}(c);
    };

    template<typename... Ts>
    constexpr auto detect_derived_archetype(archetype_storage_t<Ts...>&&)
    {
        return archetype_storage_t<Ts...>{};
    }
    template<typename T, bool> struct unpack_derived_archetype;
    template<typename T> struct unpack_derived_archetype<T, false> { using type = T; };
    template<typename T> 
    struct unpack_derived_archetype < T, true >
    {
        using type = decltype(detect_derived_archetype(T{}));
    };
    template<typename T, bool B> using unpack_derived_archetype_t = unpack_derived_archetype<T, B>::type;

    template<typename ArcheOrBehaviour>
    concept ArchetypeOrQueryBehaviorConcept =   IsArchetypeConcept<ArcheOrBehaviour>
                                             || IsDerivedFromArchetypeConcept<ArcheOrBehaviour>
                                             || (HasEachMethod<ArcheOrBehaviour>) 
                                             || (!IsTemplateClassConcept<ArcheOrBehaviour> && std::is_standard_layout_v<ArcheOrBehaviour>);

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
            (typename to_archetype_t<As>::type::template each(std::forward<Lambda>(lambda)), ...);
        }

    public:
        template<typename Lambda>
        static void each(Lambda&& lambda)
        {
            using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
            using archetype_list = search_t<mpl::list_remove_qualifier_t<args_type>>::type;
            each_impl(std::forward<Lambda>(lambda), archetype_list{});
        }

        template<typename T> struct select_behaviour_t;
        template<typename T, typename... ArcheOrBehaviour>
        struct select_behaviour_t<mpl::typelist<T, ArcheOrBehaviour...>>
        {
            using behaviour = mpl::traits::if_else_t<HasEachMethod<T>, T, mpl::nulllist>;
            using type = mpl::concat<behaviour, typename select_behaviour_t<mpl::typelist<ArcheOrBehaviour...>>::type>::type;
        };
        template<>
        struct select_behaviour_t<mpl::nulllist> { using type = mpl::nulllist; };

        template<typename... ArcheOrBehaviour>
        requires (ArchetypeOrQueryBehaviorConcept<ArcheOrBehaviour> && ...)
        struct with
        {
            using archetype_list = search_t<mpl::unpack_unpack_t<unpack_derived_archetype_t<ArcheOrBehaviour, IsDerivedFromArchetypeConcept<ArcheOrBehaviour>>...>>::type;
            using behaviour_list = select_behaviour_t< mpl::typelist<ArcheOrBehaviour...>>::type;
        private:

            template<typename TList, typename Arche> struct trait_behaviour_t;

            template<typename Arche, typename... Bs, template<typename> class BehaviourOrArche, typename... TList>
            struct trait_behaviour_t<mpl::typelist<BehaviourOrArche<Bs...>, TList...>, Arche>
            {
                using type = mpl::traits::if_else_t<mpl::is_same_v<mpl::to_list_t<mpl::at_t<0, mpl::typelist<Bs...>>>, Arche>, 
                                                    BehaviourOrArche<Bs...>, 
                                                    typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type>;
                using default_type = typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::default_type;
            };
            template<typename Arche, typename T, typename... TList>
            struct trait_behaviour_t<mpl::typelist<T, TList...>, Arche>
            {
                using type = typename trait_behaviour_t<mpl::typelist<TList...>, Arche>::type;
                //using type = mpl::typelist<TList...>;
                //using to_list = mpl::to_list_t<T>;
                using default_type = T;
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
                using behaivour_type = mpl::traits::if_else_t<mpl::is_same_v<found_type, mpl::nulllist>, typename trait_behaviour_t<behaviour_list, Arche>::default_type, found_type>;
                static_assert(!mpl::is_null_v<behaivour_type>, "you need to pass a class with static each() to template parameter of run<...>. like: template<> class something<> { each(...) {...}");
                if constexpr(mpl::is_null_v<behaivour_type>)//prevent compiler error unneed.
                { return; }
                else
                {
                    using args_type = utility::inspector_callable_t<decltype(&behaivour_type::each)>::args_type;
                    static_assert(mpl::is_sub_v<mpl::to_list_t<typename to_archetype_t<Arche>::type>, mpl::list_remove_qualifier_t<args_type>>, "args of each are not belong to selections of `run`");
                    typename to_archetype_t<Arche>::type::template each(&behaivour_type::each);
                }
            }

            template<typename Lambda, typename Arche>
            static void loop_impl(Lambda&& lambda, Arche)
            {
                using behaivour_type = trait_behaviour_t<behaviour_list, Arche>::type;
                if constexpr (mpl::is_null_v<behaivour_type>)//prevent compiler error unneed.
                {
                    using args_type = utility::inspector_callable_t<decltype(&mpl::traits::remove_qualifier_t<Lambda>::operator())>::args_type;
                    static_assert(mpl::is_sub_v<mpl::to_list_t<typename to_archetype_t<Arche>::type>, mpl::list_remove_qualifier_t<args_type>>, "args of lambda or function are not belong to selections of `run`");
                    typename to_archetype_t<Arche>::type::template each(lambda);
                    return;
                }
                else
                {
                    using args_type = utility::inspector_callable_t<decltype(&behaivour_type::each)>::args_type;
                    static_assert(mpl::is_sub_v<mpl::to_list_t<typename to_archetype_t<Arche>::type>, mpl::list_remove_qualifier_t<args_type>>, "args of each are not belong to selections of `run`");
                    typename to_archetype_t<Arche>::type::template each(&behaivour_type::each);
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
                //((std::cout << typeid(As).name() << std::endl), ...);
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
                loop(std::forward<Lambda>(lambda), archetype_list{});
            }

        };
    };


    //export template<typename... Args>
    struct hierarchy_task_t
    {
        static void run()
        {

        }
    };
}