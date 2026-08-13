export module tecs.advanced:world;

import tecs.basic;
import component_defines;
import archetype_defines;

import configs;
import mpl;
import std;

namespace tecs
{
    export template<size_t UniqueTag = 0>
    struct world_t
    {
    private:
        template<size_t N, typename = void> 
        struct archetype_instantiation_t { enum { value = 0 }; };

        template<size_t N>
        struct archetype_instantiation_t<N, mpl::void_t<decltype(sizeof(archetype_of_id_t<N>))>>
        {
            using arche_type = typename archetype_of_id_t<N>::type;
            //make sure archetype instantiation, so query can handle compile time id correctly.
            static constexpr arche_type arche = arche_type{};
            enum { value = 1 + archetype_instantiation_t<N+1>::value };
        };

    public:
        static constexpr size_t archetypes_count = archetype_instantiation_t<configs::TYPE_ID_START_INDEX>::value;

    private:

    public:

        /// <summary>
        /// world::init must be call before any interface/class/function of tecs.
        /// </summary>
        static void init()
        {
            //std::cout << "archetypes_count " << archetypes_count << std::endl;
        }
    };
}
