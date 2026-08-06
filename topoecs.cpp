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
using list_list = mpl::typelist<list1, list2, list3, list4>;
using list_list_12 = mpl::typelist<list1, list2>;
using list_list_34 = mpl::typelist<list3, list4>;
using list_list_list = mpl::typelist<list_list>;
using list_sub = mpl::list_has_sub<list_list, mpl::typelist<char, float>>::type;
using list_list_unpack = mpl::unpack<list_list>::type;

struct AAA { int a = 0; int b = 0; };

template< typename t, std::size_t n, typename = void >
struct function_argument_type;

template< typename r, typename ... a, std::size_t n >
struct function_argument_type< r(*)(a ...), n >
{
    typedef typename std::tuple_element< n, std::tuple< a ... > >::type type;
};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_argument_type< r(c::*)(a ...), n >
    : function_argument_type< r(*)(a ...), n > {
};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_argument_type< r(c::*)(a ...) const, n >
    : function_argument_type< r(c::*)(a ...), n > {
};

template< typename ftor, std::size_t n >
struct function_argument_type< ftor, n,
    typename std::conditional< false, decltype(&ftor::operator ()), void >::type >
    : function_argument_type< decltype(&ftor::operator ()), n > {
};


void update(tecs::entity_t ent, const position_t& p)
{
    std::cout << std::format("{}", ent) << std::endl;
    std::cout << "update " << p.x << p.y << p.z << std::endl;
}

int main()
{
    tecs::world_t<>::init();

    auto ent1 = hurtable_movable_anim_unit_t::create(position_t{ 1,2,3 });
    std::cout << std::format("{}", ent1) << std::endl;
    auto ent2 = hurtable_movable_anim_unit_t::create(position_t{ 2,3,4 });
    std::cout << std::format("{}", ent2) << std::endl;
    //position_t p{};
    //position_t& pos = p;
    //transform_t::get(ent, pos);
    //std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;
    int a = 20;
    auto lam = [](tecs::entity_t ent, position_t& p) 
    {   
            std::cout << std::format("{}", ent) << std::endl;
        std::cout << "update " << p.x << p.y << p.z << std::endl; 
        p.x *= 3; p.y *= 3; p.z *= 3;
    };
    hurtable_movable_anim_unit_t::each(lam);
    auto lam2 = [](tecs::entity_t ent, position_t& p)
        {
            std::cout << std::format("{}", ent) << std::endl;
            std::cout << "update " << p.x << p.y << p.z << std::endl;
        };
    hurtable_movable_anim_unit_t::each(update);

    
    std::cout << helper::type_format<tecs::query_t<movment_t>::archetype_list>() << std::endl;
    std::cout << "------" << std::endl;
    tecs::query_t<tecs::entity_t, position_t>::each(lam);
    tecs::query_t<tecs::entity_t, position_t>::each(lam2);

    //std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
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
