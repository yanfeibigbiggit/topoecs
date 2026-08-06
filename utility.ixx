export module tecs.basic:utility;

import typelist;
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

        export template<typename T> struct inspector_callable_t;

        export template<typename R, typename C, typename... Ts>
        struct inspector_callable_t < R(C::*)(Ts...)>
        {
            using return_type = R;
            using args_type = mpl::typelist<Ts...>;
            //using signature = mpl::typelist<R, C, Ts...>;
        };

        export template<typename R, typename C, typename... Ts>
        struct inspector_callable_t < R(C::*)(Ts...) const>
        {
            using return_type = R;
            using args_type = mpl::typelist<Ts...>;
            //using signature = mpl::typelist<R, C, Ts...>;
        };

        export template<typename R, typename... Ts>
        struct inspector_callable_t < R(*)(Ts...)>
        {
            using return_type = R;
            using args_type = mpl::typelist<Ts...>;
            //using signature = mpl::typelist<R, C, Ts...>;
        };
    }
}
