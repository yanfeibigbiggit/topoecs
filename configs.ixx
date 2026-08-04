export module configs;

namespace tecs
{
    namespace configs
    {
        /// <summary>
        /// entity_t: id occupy bits
        /// </summary>
        export inline const size_t ID_BITS = 20;

        /// <summary>
        /// entity_t: version occupy bits
        /// </summary>
        export inline const size_t VERSION_BITS = 12;

        /// <summary>
        /// entity_t: archetype occuy bits
        /// </summary>
        export inline const size_t ARCHETYPE_BITS = 12;
        /// <summary>
        /// The maximum limit for the total number of archetypes; exceeding this limit will result in unpredictable behavior.
        /// must be power of 2, and small than ushort:max. Default:4096.
        /// </summary>
        export inline const size_t ARCHETYPES_MAX = 1 << ARCHETYPE_BITS;
        /// <summary>
        /// The memory size of a chunk holding the archetype. 
        /// The actual allocated memory will be the value closest to this size, determined by the archetype count (aligned to a power of 2).
        /// </summary>
        export inline const size_t ARCHETYPE_CHUNK_LENGTH = 8 * 1024;
    }

};
