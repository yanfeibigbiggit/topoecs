export module archetype_defines;

import core;
import componnet_defines;

export using transfrom_t = tecs::archetype_t<position_t, rotation_t, scale_t>;
export using unit_t = tecs::archetype_t<attribute_base_t, attribute_advance_t>;