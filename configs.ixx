export module configs;

namespace tecs
{
    namespace configs
    {
        /// <summary>
        /// The maximum limit for the total number of archetypes; exceeding this limit will result in unpredictable behavior.
        /// </summary>
        export inline const unsigned int ARCHETYPE_TYPE_MAX_COUNT = 5;
        /// <summary>
        /// The memory size of a chunk holding the archetype. 
        /// The actual allocated memory will be the value closest to this size, determined by the archetype count (aligned to a power of 2).
        /// </summary>
        export inline const unsigned int ARCHETYPE_CHUNK_LENGTH = 4 * 1024;
    }

};
