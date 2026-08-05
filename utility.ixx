export module tecs:utility;

import std;

namespace tecs
{
    namespace utility
    {
        export template <typename T, std::size_t N>
        constexpr std::array<T, N> make_array(const T& value)
        {
            return[]<std::size_t... Is>(std::index_sequence<Is...>, const T & val)
            {
                return std::array<T, N>{ {(static_cast<void>(Is), val)...} };
            }(std::make_index_sequence<N>{}, value);
        }
    }
}
