export module tecs:archetype_defines;
//#include "configs_macro.h"

import std;

import :core;
import :component_defines;
import :misc;

//export inline constexpr _name  __inline_##_name = _name{};

#define REGISTER_STATIC_ARCHETYPE(_name) export template<> struct tecs::archetype_of_id_t<_name::this_id> { using type = _name; };

////////////////////////////////////////////////////////////////
// add archetypes here.
// examples
////////////////////////////////////////////////////////////////
export using transform_t = tecs::archetype_t<position_t, rotation_t, scale_t>;
REGISTER_STATIC_ARCHETYPE(transform_t);

export using hurtable_unit_t = tecs::archetype_t<attribute_base_t, attribute_advance_t>;
REGISTER_STATIC_ARCHETYPE(hurtable_unit_t);

export using movable_unit_t = tecs::archetype_t<transform_t, movment_t>;
REGISTER_STATIC_ARCHETYPE(movable_unit_t);

export using hurtable_movable_unit_t = tecs::archetype_t<movable_unit_t, hurtable_unit_t>;
REGISTER_STATIC_ARCHETYPE(hurtable_movable_unit_t);

export using hurtable_movable_anim_unit_t = tecs::archetype_t<hurtable_movable_unit_t, animation_t>;
REGISTER_STATIC_ARCHETYPE(hurtable_movable_anim_unit_t);

//
//template<> struct tecs::register_archetype_t<transform_t, hurtable_unit_t, movable_unit_t, hurtable_movable_unit_t, hurtable_movable_anim_unit_t> {};
//export using archetype_list_t = tecs::register_archetype_t<transform_t, hurtable_unit_t, movable_unit_t, hurtable_movable_unit_t, hurtable_movable_anim_unit_t>;