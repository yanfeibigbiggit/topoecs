This is a static ECS library based on C++23; it requires module support.

steps:

#### 01 Define your components in components.ixx:

```c++
export module tecs.datas:components;

import tecs.basic;
import std;

export struct position_t
{
    float x; float y; float z;
};
export struct rotation_t
{
    float u; float v; float w; float y;
};
export struct scale_t
{
    float x; float y; float z;
};
//......
```

#### 02 Define your archetypes in archetypes.ixx:

```c++
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

export struct hurtable_movable_unit_t : tecs::archetype_t<movable_unit_t, hurtable_unit_t>::default_t{};
REGISTER_STATIC_ARCHETYPE(hurtable_movable_unit_t);
```

#### 03 Create archetypes:

```c++
for (int i = 0; i < 1000000; i++)
{
    auto ent = movable_unit_t::create(movment_t{ 1,2 }, position_t{ 1,2,3 }, rotation_t{ 2,3,4 }, scale_t{ 3,4,5 });
    ent = hurtable_movable_unit_t::create(position_t{ 7,7,7 }, movment_t{ 3,3 }, attribute_base_t{5, 5});
}
```

#### 04 Loop your archetype:

A. Simplest way.

```C++
// by `each`  interface of archetype itself:
movable_unit_t::each([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t& r)
{
    std::cout << "position_t.x " << pos.x << " position_t.y " << pos.y << " position_t.z " << pos.z << std::endl;
});
```

B.  Using query_t::with.

query_t::with<A, B> means that find all archetypes contain all components of A and B both.

```c++
//find all archetypes that contain components of movable_unit_t
tecs::query_t::with<movable_unit_t>::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t & r)
{
    pos.x += pos.x + m.speed + r.u;
});
//find all archetypes that contain components of movable_unit_t and renderable_t
tecs::query_t::with<movable_unit_t, renderable_t>::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t & r)
{
    pos.x += pos.x + m.speed + r.u;
});
```

#### 05 Loop with static polymorphism:

```c++
//define behaviour of movable_unit_t:
template<typename... T> struct jump_behaviour;
template<>
struct jump_behaviour<hurtable_movable_unit_t> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent, movment_t& m)
    {
        std::cout << "I AM SPECIAL " << m.speed << std::endl;
    }
};

//archetypes that contain moveable_unit_t run with default lambda,
//but hurtable_movable_unit_t will be override
tecs::query_t::with<movable_unit_t, 
                   jump_behaviour<hurtable_movable_unit_t>
                   >::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t & r)
{
	std::cout << std::format("run default lambda: {} {}", ent, pos) << std::endl;
});

//loop all arhcetypes contains (movment_t&, position_t&, rotation_t &)
//but hurtable_movable_unit_t will be override
tecs::query_t::with<
                    ump_behaviour<hurtable_movable_unit_t>
                    >::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t & r)
{
	std::cout << std::format("run default lambda: {} {}", ent, pos) << std::endl;
});
```

#### 06 init:
```c++
//must be called before any thing in your program.
//main()
tecs::world_t<>::init();
```



