export module tecs.datas:archetypes;

import tecs.basic;
import :components;

import std;

export template<std::size_t ID> struct archetype_of_id_t;

#define REGISTER_STATIC_ARCHETYPE(_name) export template<> struct archetype_of_id_t<_name::this_id> { using type = _name; };



////////////////////////////////////////////////////////////////
// add archetypes here.
// examples
////////////////////////////////////////////////////////////////

//export using transform_t = tecs::archetype_global_t<position_t, rotation_t, scale_t>;
//REGISTER_STATIC_ARCHETYPE(transform_t);

export struct transform_t : tecs::archetype_t<position_t, rotation_t, scale_t>::default_t {};
REGISTER_STATIC_ARCHETYPE(transform_t);

export struct renderable_t : tecs::archetype_t<render_t, material_t>::default_t{};
REGISTER_STATIC_ARCHETYPE(renderable_t);

export struct hurtable_unit_t : tecs::archetype_t<attribute_base_t, attribute_advance_t>::default_t {};
REGISTER_STATIC_ARCHETYPE(hurtable_unit_t);

export struct movable_unit_t : tecs::archetype_t<transform_t, movment_t>::default_t{};
REGISTER_STATIC_ARCHETYPE(movable_unit_t);

export struct movable_renderable_t : tecs::archetype_t<movable_unit_t, renderable_t>::default_t{};
REGISTER_STATIC_ARCHETYPE(movable_renderable_t);

//export using movable_unit_renderable_t = tecs::archetype_default_t<movable_renderable_t, hurtable_unit_t>;
//REGISTER_STATIC_ARCHETYPE(movable_unit_renderable_t);

export struct hurtable_movable_unit_t : tecs::archetype_t<movable_unit_t, hurtable_unit_t>::default_t{};
REGISTER_STATIC_ARCHETYPE(hurtable_movable_unit_t);

