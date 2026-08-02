export module tecs:mpl_std;

import std;

using stdstring = std::string;

namespace mpl
{
    //>>>>>>>> is_std_pair_v
    template <typename> struct is_std_pair : std::false_type {};

    template <typename T, typename U> struct is_std_pair<std::pair<T, U>> : std::true_type {};

    export template<typename T>
        inline constexpr bool is_std_pair_v = is_std_pair<T>::value; 
    //<<<<<<<<
}
