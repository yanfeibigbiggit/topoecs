export module archetype_defines;

import core;
import component_defines;

export using transfrom_t = tecs::archetype_t<position_t, rotation_t, scale_t>;
export using transfrom_t2 = tecs::archetype_t<position_t, scale_t, rotation_t>;
export using transfrom_t3 = tecs::archetype_t<rotation_t, position_t, scale_t>;
export using transfrom_t4 = tecs::archetype_t<rotation_t, scale_t, position_t>;
export using hurtable_unit_t = tecs::archetype_t<attribute_base_t, attribute_advance_t>;
export using hurtable_unit_t2 = tecs::archetype_t<attribute_advance_t, attribute_base_t>;