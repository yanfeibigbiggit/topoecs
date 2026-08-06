export module tecs.basic:maths;

namespace tecs
{
	namespace maths
	{
		export constexpr size_t round_power_2(size_t v)
		{
			v = v <= 0 ? 1 : v;
			int x = 1;
			while (x <= v) { x *= 2; }
			return (x - v < v - x / 2) ? x : x / 2;
		}

		//export constexpr void max(size_t a, size_t b)
		//{
		//	
		//}
	}
}