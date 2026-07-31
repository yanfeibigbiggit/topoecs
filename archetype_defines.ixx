export module archetype_defines;

import std;
import core;
import component_defines;

#define BEGIN_STATIC_ARCHETYPE_PROCCESS() export template<std::size_t ID> struct id_to_archetype_t;
#define REGISTER_STATIC_ARCHETYPE(_name) export template<> struct id_to_archetype_t<tecs::misc::type_id_archetype<_name>> { using type = _name; };


BEGIN_STATIC_ARCHETYPE_PROCCESS();
//export template<std::size_t ID> struct id_to_archetype_t;

export using transform_t = tecs::archetype_t<position_t, rotation_t, scale_t>;
REGISTER_STATIC_ARCHETYPE(transform_t);

export using hurtable_unit_t = tecs::archetype_t<attribute_base_t, attribute_advance_t>;
REGISTER_STATIC_ARCHETYPE(hurtable_unit_t);


//export using transform_t2 = tecs::archetype_t<position_t, scale_t, rotation_t>;
//export using transform_t3 = tecs::archetype_t<rotation_t, position_t, scale_t>;
//export using transform_t4 = tecs::archetype_t<rotation_t, scale_t, position_t>;