export module helper;
import std;

using xstring = std::string;

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
    inline xstring print_type()
    {
        xstring name = typeid(T).name();
        return replace_all(name, "struct ", "");
    }
}



