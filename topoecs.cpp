// topoecs.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//
import std;
import core;
import component_defines;
import archetype_defines;


int main()
{
    //std::cout << transform_t::this_id << " -- " << hurtable_unit_t::assure_type_mapping << std::endl;
    //tecs::misc::type_map_t<attribute_base_t>::push< hurtable_unit_t::this_id>();
    //std::cout << "has<transform_t::this_id>: " << tecs::misc::type_map_t<position_t>::has<transform_t::this_id>() << std::endl;
    //std::cout << "has<fightable_unit_t::this_id>: " << tecs::misc::type_map_t<attribute_base_t>::has<hurtable_unit_t::this_id>() << std::endl;

    //std::cout << "type_id_archetype<transform_t> " << typeid(id_to_archetype_t<tecs::misc::type_id_archetype<transform_t>>::type).name() << std::endl;
    //std::cout << "type_id_archetype<hurtable_unit_t> " << typeid(id_to_archetype_t<tecs::misc::type_id_archetype<hurtable_unit_t>>::type).name() << std::endl;
   
    //std::cout << typeid(transform_t).name() << std::endl;
    //std::cout << typeid(transform_t2).name() << std::endl;
    //std::cout << typeid(transform_t3).name() << std::endl;
    //std::cout << typeid(transform_t4).name() << std::endl;

    //transform_t::set<position_t>(position_t{1, 2, 3});
    //position_t p = transform_t::get<position_t>();
    //std::cout << p.x << " " << p.y << " " << p.z << std::endl;

    //using t1 = tecs::archetype_t<int, char, float>;
    //std::cout << typeid(t1).name() << std::endl;
    //using t2 = tecs::archetype_t<int, float, char>;
    //std::cout << typeid(t2).name() << std::endl;
    //using t3 = tecs::archetype_t<float, int, char>;
    //std::cout << typeid(t3).name() << std::endl;

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
