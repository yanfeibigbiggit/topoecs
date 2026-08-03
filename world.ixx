export module tecs:world;

import :misc;
import :archetype_defines;

import configs;
import mpl;

namespace tecs
{
    export template<typename UniqueTag>
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

        template<>
        struct archetype_instantiation_t<configs::ARCHETYPE_TYPE_MAX_COUNT> { enum {value =  0 }; };

    public:
        static constexpr size_t archetypes_count = archetype_instantiation_t<0>::value;
    public:
        /// <summary>
        /// world::init must be call before any interface/class/function of tecs.
        /// </summary>
        static constexpr void init()
        {
            std::cout << "archetypes_count " << archetypes_count << std::endl;
        }
    };
}
