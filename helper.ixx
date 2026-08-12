export module helper;

import tecs.basic;
import component_defines;

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
struct std::formatter<tecs::entity_t>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin();}

    auto format(const tecs::entity_t& ent, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "entity:({:07d} {:07d} {:07d})", (unsigned int)(ent.get_id()), (unsigned int)(ent.get_version()), (unsigned int)(ent.get_arche()));
    }
};

template<>
struct std::formatter<position_t>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const position_t& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "position:({} {} {})", t.x, t.y, t.z);
    }
};

template<>
struct std::formatter<rotation_t>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const rotation_t& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "rotation:({} {} {} {})", t.u, t.v, t.w, t.y);
    }
};

template<>
struct std::formatter<attribute_base_t>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const attribute_base_t& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "attr_base:({} {} {} {} {} {})", t.strength, t.agility, t.intelligence, t.conititution, t.wistom, t.spirit);
    }
};


