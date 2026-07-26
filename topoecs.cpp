// topoecs.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

import std;
import core;

int main()
{

    tecs::component_t::define("position", "x", tecs::basic_type_t::float32, "y", tecs::basic_type_t::float32, "z", tecs::basic_type_t::float64);
    tecs::component_t::define("life", "value", tecs::basic_type_t::int32);

    tecs::archetype_t arche;
    arche.add("position", "life");
    std::cout << arche.format_string() << std::endl;

    std::array<const char*, 2> compvector = { "animation", "audio" };
    tecs::archetype_t::define(compvector.begin(), compvector.end());
    std::map<int,  std::string> map = { {1, "counter"}, {2, "timer"} };
    tecs::archetype_t::define(map.begin(), map.end());

    const char* comparray[] = { "renderer", "physics" };
    tecs::archetype_t::define(comparray, 0, 2);

    tecs::archetype_t::define("1111", "2222", "3333");

    //constexpr auto tname = tecs::caller::type_name_id<int>();
    //std::cout << tname << std::endl;
    //tecs::caller::type_name_id<char>();
    //tecs::caller::type_name_id<float>();

    constexpr auto r1 = tecs::caller::sort_type_sequence<int, char, float>();
    std::cout << typeid(decltype(r1)).name() << std::endl;
    constexpr auto r2 = tecs::caller::sort_type_sequence<int, float, char>();
    std::cout << typeid(decltype(r2)).name() << std::endl;
    constexpr auto r3 = tecs::caller::sort_type_sequence<float, char, int>();
    std::cout << typeid(decltype(r3)).name() << std::endl;


    //auto k1 = tecs::caller::sort_type_sequence_test<int, char, float>();
    //for (auto i : k1) { std::cout << i << " "; } std::cout << std::endl;
    //auto k2 = tecs::caller::sort_type_sequence_test<int, float, char>();
    //for (auto i : k2) { std::cout << i << " "; } std::cout << std::endl;

    //auto t1 = tecs::caller::sort_type_sequence_test<int, char, float>();
    //std::cout << typeid(decltype(t1)).name() << std::endl;
    //auto t2 = tecs::caller::sort_type_sequence_test<int, float, char>();
    //std::cout << typeid(decltype(t2)).name() << std::endl;

    //auto t1 = tecs::caller::sort_index_sequence_test<int, char, float>();
    //for (auto i : t1) { std::cout << i.first << " " << i.second << " "; } std::cout << std::endl;
    //auto t2 = tecs::caller::sort_index_sequence_test<int, float, char>();
    //for (auto i : t2) { std::cout << i.first << " " << i.second << " "; } std::cout << std::endl;
    //std::cout << tecs::caller::type_id<int> << " " << tecs::caller::type_id<char> << " " << tecs::caller::type_id<float> << std::endl;



    //auto v1 = tecs::caller::sort_index_sequence<int, char, float>();
    //for (auto i : v1) { std::cout << i << " "; } std::cout << std::endl;
    //auto v2 = tecs::caller::sort_index_sequence<int, float, char>();
    //for (auto i : v2) { std::cout << i << " "; } std::cout << std::endl;

    //std::array<int, 3> x1{ 0, 1, 2 };
    //std::array<int, 3> x2{ 4, 2, 1 };
    //std::ranges::sort(x1, [&](int l, int r) { return x2[l] <= x2[r]; });
    //for(int i : x1)
    //    std::cout << i << std::endl;

    //std::cout << tecs::caller::sort_type_sequence<int, char, float>() << std::endl;
    //std::cout << tecs::caller::sort_type_sequence<int, float, char>() << std::endl;
    //std::cout << tecs::caller::sort_type_sequence<char, float, int>() << std::endl;
    //std::cout << tecs::caller::sort_type_sequence<char, int, float>() << std::endl;
    //std::cout << tecs::caller::sort_type_sequence<float, int, char>() << std::endl;
    //std::cout << tecs::caller::sort_type_sequence<float, char, int>() << std::endl;

    //constexpr tecs::caller::type_id_t cc1 = tecs::caller::type_id<int, char, float>;
    //std::cout << cc1 << std::endl;
    //constexpr tecs::caller::type_id_t cc2 = tecs::caller::type_id<int, float, char>;
    //std::cout << cc2 << std::endl;
    //constexpr tecs::caller::type_id_t cc3 = tecs::caller::type_id<char, float, int>;
    //std::cout << cc3 << std::endl;
    //constexpr tecs::caller::type_id_t cc4 = tecs::caller::type_id<char, int, float>;
    //std::cout << cc4 << std::endl;
    //constexpr tecs::caller::type_id_t cc5 = tecs::caller::type_id<float, int, char>;
    //std::cout << cc5 << std::endl;
    //constexpr tecs::caller::type_id_t cc6 = tecs::caller::type_id<float, char, int>;
    //std::cout << cc6 << std::endl;


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
