export module tecs:component_defines;
//#include "configs_macro.h"

import :core;

export struct entity_t
{
    int id;
    int version;
};

export struct position_t
{
    float x;
    float y;
    float z;
};

export struct rotation_t
{
    float u;
    float v;
    float w;
    float y;
};

export struct scale_t
{
    float x;
    float y;
    float z;
};

export struct attribute_base_t
{
    int strength;
    int agility;
    int intelligence;
    int conititution;
    int wistom;
    int spirit;
};

export struct attribute_advance_t
{
    int hp;
    int mana;
    int attack_physic;
    int attack_magic;
    int defence_physic;
    int defence_elements;
};

export struct skinrender_t
{
    int share_mesh;
};

export struct vertexrender_t
{
    int share_mesh;
};

export struct movment_t
{
    int speed;
};

export struct animation_t
{
    int clip;
};
