export module tecs.basic:threads;

import std;

namespace tecs
{
	class spinlock_t
	{
	public:
		std::atomic_flag flag;

		void lock(const std::memory_order order_ = std::memory_order::memory_order_seq_cst)
		{
			while (flag.test_and_set(order_));
		}
		void unlock()
		{
			flag.clear();
		}
	};
}