#ifndef ILYTH_MACROS_H
#define ILYTH_MACROS_H

#include <cassert>
#include <cstdlib>
#include <limits>
#include <print>
#include <source_location>
#include <string_view>

namespace ilyth
{

inline void assert_fail(
    std::string_view cond_str,
    std::string_view info,
    const std::source_location& loc = std::source_location::current())
{
    std::print(stderr, "{}({}:{}) Assert failed: {}\n", loc.file_name(),
               loc.line(), loc.column(), cond_str);
    if (!info.empty())
    {
        std::print(stderr, "  With info: {}\n", info);
    }
    std::print(stderr, "  Stack trace: {}\n", "TODO");
#ifdef ILYTH_DEBUG
    std::abort();  // or std::terminate()
#endif
}

#define ilyth_assert(cond)                   \
    do                                       \
    {                                        \
        if (!(cond)) [[unlikely]]            \
        {                                    \
            ::ilyth::assert_fail(#cond, {}); \
        }                                    \
    } while (false)

#define ilyth_assert2(cond, info)                \
    do                                           \
    {                                            \
        if (!(cond)) [[unlikely]]                \
        {                                        \
            ::ilyth::assert_fail(#cond, (info)); \
        }                                        \
    } while (false)

namespace types
{

using u8  = uint8_t;
using s8  = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

constexpr auto max_u8  = std::numeric_limits<u8>::max();
constexpr auto max_u16 = std::numeric_limits<u16>::max();
constexpr auto max_u32 = std::numeric_limits<u32>::max();
constexpr auto max_u64 = std::numeric_limits<u64>::max();

}  // namespace types

}  // namespace ilyth

#endif
