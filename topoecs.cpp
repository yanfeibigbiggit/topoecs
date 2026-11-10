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
        std::cout << "jump_behaviour<renderable_t>::each " << std::endl;
    }
};

template<>
struct jump_behaviour<hurtable_movable_unit_t> : tecs::behaviour_base_t
{
    static void each(tecs::entity_t ent, movment_t& m)
    {
        std::cout << "jump_behaviour<hurtable_movable_unit_t>::each " << std::endl;
    }
};

int looptime = 1000000;

namespace timer = std::chrono;
int main()
{
    tecs::world_t<>::init();

    double sum = 0;
    std::vector<tecs::entity_t> entities;
    timer::steady_clock::time_point create_start = timer::steady_clock::now();
    for (int i = 0; i < looptime; i++)
    {
        auto ent = movable_unit_t::create(movment_t{ 1,2 }, position_t{ 1,2,3 }, rotation_t{ 2,3,4 }, scale_t{ 3,4,5 });
        entities.emplace_back(ent);
        ent = hurtable_movable_unit_t::create(position_t{ 7,7,7 }, movment_t{ 3,3 }, attribute_base_t{ 5, 5 });
        //entities.emplace_back(ent);
    }
    timer::steady_clock::time_point create_end = timer::steady_clock::now();
    timer::duration<double> duration = timer::duration_cast<timer::duration<double>>(create_end - create_start);
    std::cout << "create time " << duration.count() << std::endl;

    //for (int i = looptime-10; i < looptime; i++)
    //{
    //    auto ent = entities[i];
    //    //std::cout << std::format("{} {}", movable_unit_t::get<tecs::entity_t>(ent), movable_unit_t::get_ref<rotation_t>(ent)) << std::endl;
    //    //movable_unit_t::remove(ent);
    //}

    create_start = timer::steady_clock::now();
    tecs::query_t::with<movable_unit_t, jump_behaviour<movable_unit_t> /*, jump_behaviour<>*/>::run([](tecs::entity_t ent, movment_t& m, position_t& pos, rotation_t& r)
        {
            //std::cout << std::format("run default lambda: {} {}", ent, pos) << std::endl;
            pos.x += pos.x + m.speed + r.u;
            //m.direction = m.speed + ent.get_id();
        });
    create_end = timer::steady_clock::now();
    duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    std::cout << "update time " << duration.count() << std::endl;

    //double sum = 0;
    //timer::steady_clock::time_point create_start = timer::steady_clock::now();
    //for (int i = 0; i < looptime; i++)
    //{
    //    auto ent = hurtable_movable_unit_t::create(position_t{ 1,2,3 }, rotation_t{10,20,30 }, scale_t{ 1,1,1 }/*, attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 }*/);
    //}
    //timer::steady_clock::time_point create_end = timer::steady_clock::now();
    //timer::duration<double> duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    //std::cout << "create time " << duration.count() << std::endl;

    //create_start = timer::steady_clock::now();
    //hurtable_movable_unit_t::each([](tecs::entity_t ent, position_t& pos, const rotation_t rot/*, const attribute_base_t ab, const movment_t m*/)
    //    {
    //        std::cout << std::format("{} {}", ent, pos) << std::endl;
    //        //pos.x += pos.y + rot.u;
    //    });
    ////hurtable_movable_unit_t::get(tecs::entity_t(8, 0, 0, 11), position_)
    //create_end = timer::steady_clock::now();
    //duration = timer::duration_cast<timer::duration<double>> (create_end - create_start);
    //std::cout << "update time " << duration.count() << std::endl;
    //std::cout << "sum " << sum << std::endl;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
