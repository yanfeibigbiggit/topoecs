export module component_defines;

import tecs.basic;

import std;

export struct at {}; export struct bt {}; export struct ct {}; export struct dt {}; export struct et {}; export struct ft {}; export struct gt {}; export struct ht {};

export struct position_t
{
    float x;
    float y;
    float z;
    //position_t() {}
    //position_t(int xx, int yy, int zz) :x(xx), y(yy), z(zz) { std::cout << "position_t cons" << std::endl; }
    //position_t(position_t&& other):x(other.x),y(other.y),z(other.z){ std::cout << "position_t move" << std::endl; }
    //position_t& operator=(position_t& other) { x = other.x; y = other.y; z = other.z; return *this; std::cout << "position_t =&" << std::endl; }
    //position_t& operator=(position_t&& other) { x = other.x; y = other.y; z = other.z; return *this; std::cout << "position_t =&&" << std::endl; }
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

export struct render_t
{
    int share_mesh;
    int mat;
};

export struct material_t
{
    int texture;
    int color;
};

export struct texture_t
{
    int width;
    int height;
    int address;
};

export struct movment_t
{
    int speed;
    int direction;
};

export struct animation_t
{
    int clip;
    int enter_blend;
    int enter_time;
    int exit_blend;
    int exit_time;
};
