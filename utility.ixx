export module tecs.basic:utility;

import typelist;
import std;

namespace tecs
{
    namespace utility
    {
        //////////////////////////////////////////////////////////////////////
        export template <typename T, std::size_t N>
        constexpr std::array<T, N> make_array(const T& value)
        {
            return[]<std::size_t... Is>(std::index_sequence<Is...>, const T & val)
            {
                return std::array<T, N>{ {(static_cast<void>(Is), val)...} };
            }(std::make_index_sequence<N>{}, value);
        }

        //////////////////////////////////////////////////////////////////////
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

        //////////////////////////////////////////////////////////
        //data_size_of_t
        //////////////////////////////////////////////////////////
        template<typename... Args> struct data_size_of_t;

        template<typename T, typename... Args>
        struct data_size_of_t<T, Args...>
        {
            enum { value = sizeof(T) + data_size_of_t<Args...>::value };
        };

        template<typename T>
        struct data_size_of_t<T>
        {
            enum { value = sizeof(T) };
        };


        //////////////////////////////////////////////////////////////////////
        //template<typename T>
        //struct HasMethod {
        //    static constexpr bool value = false;
        //};

        //// Non-static member function pointer
        //template<typename R, typename C, typename... A>
        //struct HasMethod<R(C::*)(A...)> {
        //    static constexpr bool value = true;
        //};

        //// Static member function pointer (regular function pointer)
        //template<typename R, typename... A>
        //struct HasMethod<R(*)(A...)> {
        //    static constexpr bool value = true;
        //};

        //---- example ----//
        //template<typename T>
        //concept HasEachMethod = requires { typename HasMethod<decltype(&T::each)>; }&& HasMethod<decltype(&T::each)>::value;
    }
}
