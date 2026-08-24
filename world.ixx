export module tecs.advanced:world;

import typelist;
import tecs.basic;
import tecs.datas;

import helper;
import configs;
import mpl;
import std;

namespace tecs
{
    struct build_global_archetype_info
    {
        template<typename... Arche> struct archetypes_to_ids;
        template<typename... TList>
        struct archetypes_to_ids<mpl::typelist<TList...>>
        {
            using type = mpl::seq::indexlist<TList::this_id...>;
        };
        template<> struct archetypes_to_ids<> { using type = mpl::seq::nulllist; };

        template<size_t N, typename Global, typename Prev, typename = void> 
        struct build_archetype_ids_of_global
        {
            using type = mpl::typelist<Global, typename archetypes_to_ids<Prev>::type>;
        };
        template<size_t N, typename Global, typename Prev>
        struct build_archetype_ids_of_global<N, Global, Prev, std::void_t<decltype(sizeof(archetype_of_id_t<N>))>>
        {
            using arche_type = typename archetype_of_id_t<N>::type;
            static constexpr inline bool has = mpl::has_v<arche_type::global_list, Global>;
            using pass_type = mpl::traits::if_else_t<has,
                                                     mpl::push_front_t<arche_type, Prev>,
                                                     Prev>;
            using type = typename build_archetype_ids_of_global<N + 1, Global, pass_type>::type;
        };

        template<size_t N, typename Prev, typename = void> struct apply_t { using type = Prev; };

        template<size_t N, typename Prev>
        struct apply_t < N, Prev, std::void_t<decltype(sizeof(archetype_of_id_t<N>)) >>
        {
            using arche_type = typename archetype_of_id_t<N>::type;
            using pass_type = mpl::traits::if_else_t<is_global_archetype_v<arche_type>,
                                                    mpl::push_front_t<typename build_archetype_ids_of_global<configs::TYPE_ID_START_INDEX, arche_type, mpl::nulllist>::type, Prev>,
                                                    Prev>;

            using type = typename apply_t<N + 1, pass_type>::type;
        };

        using type = typename apply_t<configs::TYPE_ID_START_INDEX, mpl::nulllist, void>::type;
    };

    export template<size_t UniqueFlag = 0>
    struct world_t
    {
    private:
        template<size_t N, typename = void> 
        struct archetype_instantiation_t { enum { value = 0 }; };

        template<size_t N>
        struct archetype_instantiation_t<N, std::void_t<decltype(sizeof(archetype_of_id_t<N>))>>
        {
            using arche_type = typename archetype_of_id_t<N>::type;
            //make sure archetype instantiation, so query can handle compile time id correctly.
            //static constexpr arche_type arche = arche_type{};
            enum { arche_id = arche_type::this_id };
            enum { value = 1 + archetype_instantiation_t<N+1>::value };
        };

    public:
        static constexpr size_t archetypes_count = archetype_instantiation_t<configs::TYPE_ID_START_INDEX>::value;

    private:
        template<size_t N, typename GlobalInfoList>
        static void set_global_archetype_sparse_indexer()
        {
            constexpr size_t len = mpl::length<GlobalInfoList>::value;
            if constexpr(N >= len)
            { return; }
            else
            {
                using GlobalInfo = mpl::at_t<N, GlobalInfoList>;
                using Arche = mpl::at_t<0, GlobalInfo>;
                using ids = mpl::at_t<1, GlobalInfo>;
                Arche::make_sparse_indexer_ptr(ids{});
                set_global_archetype_sparse_indexer<N+1, GlobalInfoList>();
            }
        }

    public:

        /// <summary>
        /// world::init must be call before any interface/class/function of tecs.
        /// </summary>
        static void init()
        {
            std::cout << "archetypes_count " << archetypes_count << std::endl;
        }
    };
}
