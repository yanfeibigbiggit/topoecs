// topoecs.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
import tecs.basic;
import tecs.advanced;
import std;
import tecs.datas;
import helper;
import typelist;
import mpl;

template<typename... T> struct jump_behaviour;

template<>
struct jump_behaviour<> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent)
    {
        std::cout << "jump_behaviour<>::each " << std::endl;
    }
};
template<>
struct jump_behaviour<movable_unit_t> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t& rot, scale_t& sc)
    {
        //std::cout << "jump_behaviour<movable_unit_t>::each " << std::format("{} {} {}", pos, rot, sc) << std::endl;
        m.speed = m.direction + 1;
    }
};

template<>
struct jump_behaviour<renderable_t> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent, movment_t& m)
    {
        //std::cout << "jump_behaviour<renderable_t>::each " << std::endl;
    }
};

template<>
struct jump_behaviour<hurtable_movable_unit_t> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent, movment_t& m)
    {
        //std::cout << "jump_behaviour<hurtable_movable_unit_t>::each " << std::endl;
    }
};

int looptime = 1000000;

namespace timer = std::chrono;
int main()
{
    tecs::world_t<>::init();

    double sum = 0;
    timer::steady_clock::time_point create_start = timer::steady_clock::now();
    for (int i = 0; i < looptime; i++)
    {
        auto ent = movable_unit_t::create(movment_t{ 1,2 }, position_t{ 1,2,3 }, rotation_t{ 2,3,4 }, scale_t{ 3,4,5 });
        ent = hurtable_movable_unit_t::create(position_t{ 7,7,7 }, movment_t{ 3,3 }, attribute_base_t{ 5, 5 });
    }
    timer::steady_clock::time_point create_end = timer::steady_clock::now();
    timer::duration<double> duration = timer::duration_cast<timer::duration<double>>(create_end - create_start);
    std::cout << "create time " << duration.count() << std::endl;

    create_start = timer::steady_clock::now();
    tecs::query_t::with<movable_unit_t, jump_behaviour<hurtable_movable_unit_t> /*, jump_behaviour<>*/>::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t& r)
    {
        //std::cout << std::format("run default lambda: {} {}", ent, pos) << std::endl;
        pos.x += pos.x + m.speed + r.u;
    });
    create_end = timer::steady_clock::now();
    duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    std::cout << "update time " << duration.count() << std::endl;
}
