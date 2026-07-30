export module componnet_defines;


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
    int defence_physic;
    int defence_elements;
};
