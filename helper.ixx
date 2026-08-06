export module helper;

import tecs.basic;

import std;

using xstring = std::string;
using xstrview = std::string_view;

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

template<>
struct std::formatter<tecs::entity_t> : std::formatter<xstrview>
{
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const tecs::entity_t& ent, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "entity:({:07d} {:07d} {:07d})", (unsigned int)(ent.get_id()), (unsigned int)(ent.get_version()), (unsigned int)(ent.get_arche()));
    }

    //auto format(const tecs::entity_t& ent, std::format_context& ctx) const {
    //    xstring temp;
    //    std::format_to(std::back_inserter(temp), "({}, {}, {})", (unsigned int)(ent.get_id()), (unsigned int)(ent.get_version()), (unsigned int)(ent.get_arche()));
    //    return std::formatter<xstrview>::format(temp, ctx);
    //}
};




