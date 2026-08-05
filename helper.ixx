export module helper;
import std;

using xstring = std::string;

/// <summary>
/// for user
/// </summary>
export namespace helper
{
    inline xstring replace_all(xstring source, xstring sub, xstring replace)
    {
        xstring dst = source;
        xstring::size_type pos = 0;
        while ((pos = dst.find(sub)) != xstring::npos)
        {
            dst.replace(pos, sub.length(), replace);
        }
        return dst;
    }

    export template<typename T>
    inline xstring type_format()
    {
        xstring name = typeid(T).name();
        return replace_all(name, "struct ", "");
    }
}



