// topoecs.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//
import std;
import tecs.basic;
import typelist;
import helper;

import component_defines;
import archetype_defines;
import tecs.advanced;

using sub = mpl::typelist<int, char>;
using list1 = mpl::typelist<int, char, double>;
using list2 = mpl::typelist<int, float, char>;
using list3 = mpl::typelist<char, float>;
using list4 = mpl::typelist<double, float, long>;
using list_list = mpl::typelist<list1, list1, list2, list1>;
using list_list_12 = mpl::typelist<list1, list2>;
using list_list_34 = mpl::typelist<list3, list4>;
using list_list_list = mpl::typelist<list_list>;
using list_sub = mpl::select_sublist<list_list, mpl::typelist<char, float>>::type;
using list_list_unpack = mpl::unpack<list_list>::type;

constexpr inline size_t looptime = 1000000;


void update(tecs::entity_t ent, const position_t& p)
{
    std::cout << std::format("{}", ent) << std::endl;
    std::cout << "update " << p.x << p.y << p.z << std::endl;
}

struct custom_movable
{
    tecs::entity_t ent;
    position_t pos;
    rotation_t rot;
    scale_t scale;
    attribute_base_t ab;
    attribute_advance_t ad;
    movment_t mov;
};

template<typename... As> struct behaviour_override_t;

template<>
struct behaviour_override_t<>
{
    static void each(tecs::entity_t t, position_t p)
    {
        std::cout << "default" << std::endl;
    }
};

template<>
struct behaviour_override_t<hurtable_movable_unit_t>
{
    static void each(tecs::entity_t ent, position_t& pos, const rotation_t& rot, const attribute_base_t& ab)
    {
        std::cout << std::format("{} {} {}", ent, pos, ab) << std::endl;
    }
};

struct behaviour_union_t
{
    template<typename... Ts>
    static void each(tecs::entity_t ent, position_t& pos)
    {

    }
    template<>
    static void each<hurtable_movable_unit_t>(tecs::entity_t ent, position_t& pos)
    {

    }
};

struct AAAA
{
    static void each(tecs::entity_t t, position_t p)
    {
        std::cout << "default" << std::endl;
    }
};



struct BBBB : hurtable_movable_unit_t
{

};

template<class C> concept template_class = requires(C c)
{
    [] <template<class...> class T, class... Ts>(T<Ts...>) {}(c);
};

template <typename C>
struct is_template_class {
    static constexpr bool value = 0;
};

template <typename... Ts, template <typename> class C>
struct is_template_class<C<Ts...>> {
    static constexpr bool value = 1;
};

template <typename... T>
struct MyTemplate {};
template <>
struct MyTemplate<int, bool> {};    // some specification
template <>
struct MyTemplate<> {};             // empty specification


namespace timer = std::chrono;

int main()
{
    int sum = 0;
    tecs::world_t<>::init();

    //for (int i = 0; i < 10; i++)
    //{
    //    auto ent = hurtable_movable_unit_t::create(position_t{ 1,2,3 }, rotation_t{ 10,20,30 }, scale_t{ 1,1,1 }, attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 });
    //}

    //tecs::query_t::with<behaviour_override_t<hurtable_movable_unit_t>, behaviour_override_t<>>::run(
    //    [](tecs::entity_t ent, position_t& pos, const rotation_t& rot, const attribute_base_t& ab, const movment_t& m)
    //        {
    //            std::cout << std::format("{} {}", ent, pos) << std::endl;
    //        });

    timer::steady_clock::time_point create_start = timer::steady_clock::now();
    for (int i = 0; i < looptime; i++)
    {
        auto ent = transform_t::create(position_t{ 1,2,3 }, rotation_t{ 10,20,30 }, scale_t{ 1,1,1 }/*, attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 }*/);
    }
    timer::steady_clock::time_point create_end = timer::steady_clock::now();
    timer::duration<double> duration = timer::duration_cast<timer::duration<double >> (create_end - create_start);
    std::cout << "create time " << duration.count() << std::endl;

    create_start = timer::steady_clock::now();
    tecs::query_t::with<transform_t>::run([](tecs::entity_t ent, position_t& pos, const rotation_t rot/*, const attribute_base_t ab, const movment_t m*/)
        {
            //std::cout << std::format("{} {} {}", ent, pos, ab) << std::endl;
            pos.x += pos.y + rot.u;
        });
    create_end = timer::steady_clock::now();
    duration = timer::duration_cast<timer::duration<double>> (create_end - create_start);
    std::cout << "update time " << duration.count() << std::endl;
    std::cout << "sum " << sum << std::endl;

    //std::vector< hurtable_movable_unit_t::this_tuple> vt;
    //vt.resize(looptime);
    //create_start = timer::steady_clock::now();
    //for (int i = 0; i < looptime; i++)
    //{
    //    //auto tuple = hurtable_movable_unit_t::this_tuple(tecs::entity_t{ 1,2,3,4 }, position_t{ 1,2,3 }, rotation_t{ 10,20,30 }, scale_t{ 1,1,1 },
    //    //    attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 }, movment_t{});
    //    vt.emplace_back(tecs::entity_t{ 1,2,3,4 }, position_t{ 1,2,3 }, rotation_t{ 10,20,30 }, scale_t{ 1,1,1 },
    //           attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 }, movment_t{});
    //}
    //create_end = timer::steady_clock::now();
    //duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    //std::cout << "create tuple time " << duration.count() << std::endl;

    //create_start = timer::steady_clock::now();
    //for (int i = 0; i < looptime; i++)
    //{
    //    auto& tt = vt[i];
    //    std::get<1>(tt).x += std::get<1>(tt).x + std::get<2>(tt).u + +std::get<4>(tt).strength;
    //    //std::get<1>(vt[i]).x += std::get<1>(vt[i]).x + std::get<2>(vt[i]).u + +std::get<4>(vt[i]).strength;
    //}
    //create_end = timer::steady_clock::now();
    //duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    //std::cout << "update tuple time " << duration.count() << std::endl;

    //std::vector<custom_movable> ct;
    //ct.resize(looptime);
    //create_start = timer::steady_clock::now();
    //for (int i = 0; i < looptime; i++)
    //{
    //    ct.emplace_back(tecs::entity_t{ 1,2,3,4 }, position_t{ 1,2,3 }, rotation_t{ 10,20,30 }, scale_t{ 1,1,1 },
    //        attribute_base_t{ 3,3,3,3,3,3 }, attribute_advance_t{ 4,4,4,4 }, movment_t{});
    //}
    //create_end = timer::steady_clock::now();
    //duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    //std::cout << "create custom time " << duration.count() << std::endl;


    //create_start = timer::steady_clock::now();
    //for (int i = 0; i < looptime; i++)
    //{
    //    auto& tt = ct[i];
    //    tt.pos.x += tt.pos.x + tt.rot.u + +tt.ab.strength;
    //}
    //create_end = timer::steady_clock::now();
    //duration = timer::duration_cast<timer::duration<double >>(create_end - create_start);
    //std::cout << "update custom time " << duration.count() << std::endl;

    return 0;
}

//int main()
//{
//    tecs::world_t<>::init();
//
//    auto ent1 = hurtable_movable_anim_unit_t::create(position_t{ 1,2,3 });
//    std::cout << std::format("{}", ent1) << std::endl;
//    auto ent2 = hurtable_movable_anim_unit_t::create(position_t{ 2,3,4 });
//    std::cout << std::format("{}", ent2) << std::endl;
//    //position_t p{};
//    //position_t& pos = p;
//    //transform_t::get(ent, pos);
//    //std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;
//    int a = 20;
//    auto lam = [](tecs::entity_t ent, position_t& p) 
//    {   
//            std::cout << std::format("{}", ent) << std::endl;
//        std::cout << "update " << p.x << p.y << p.z << std::endl; 
//        p.x *= 3; p.y *= 3; p.z *= 3;
//    };
//    hurtable_movable_anim_unit_t::each(lam);
//    auto lam2 = [](tecs::entity_t ent, const position_t& p)
//        {
//            std::cout << std::format("{}", ent) << std::endl;
//            std::cout << "update " << p.x << p.y << p.z << std::endl;
//        };
//    hurtable_movable_anim_unit_t::each(update);
//
//    
//    std::cout << helper::type_format<tecs::query_t<movment_t>::archetype_list>() << std::endl;
//    std::cout << "------" << std::endl;
//    tecs::query_t<tecs::entity_t, position_t>::each(lam);
//    tecs::query_t<tecs::entity_t, position_t>::each(lam2);
//
//    //std::this_thread::sleep_for(std::chrono::seconds(2));
//
//    return 0;
//}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
