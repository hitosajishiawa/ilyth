#ifndef ILYTH_MACROS_H
#define ILYTH_MACROS_H

#include <cassert>
#include <format>
#include <limits>
#include <print>
#include <source_location>
#include <string_view>
#include <utility>

namespace ilyth
{

/**
 * @brief A partial function as `std::print(stderr, ...)`.
 *
 * @note Will print to stderr *instead of stdout*.
 *
 * @details The format parameter is `std::format_string<Args...>` on purpose,
 * not `std::string_view`: its constructor is `consteval`, so a mismatch between
 * the format string and the arguments is diagnosed at compile time, at the call
 * site -- exactly like calling `std::print` directly.
 *
 * `Args...` is deduced from the arguments only. The `format_string` alias hides
 * them behind `std::type_identity_t`, which is what keeps the format-string
 * parameter out of deduction; do not spell out `std::basic_format_string<char,
 * Args...>` instead, or the two parameters will fight over `Args`.
 */
template <typename... Args>
void iprint(std::format_string<Args...> fmt, Args&&... args)
{
    std::print(stderr, fmt, std::forward<Args>(args)...);
}

inline void assert_fail(
    std::string_view cond_str,
    std::string_view info,
    const std::source_location& loc = std::source_location::current())
{
    iprint("{}({}:{}) Assert failed: {}\n", loc.file_name(), loc.line(),
           loc.column(), cond_str);
    if (!info.empty())
    {
        iprint("  With info: {}\n", info);
    }
    iprint("  Stack trace: {}\n", "TODO");
#ifdef ILYTH_DEBUG
    std::abort();  // or std::terminate()
#endif
}

/**
 * @details Evaluates `cond`, reports a failure through `assert_fail`, and
 * returns whether it held. Control flow is deliberately left to the caller, so
 * `break`, `continue`, `return` and `goto` keep their usual meaning:
 * ```
 *     if (!ilyth_check(p != nullptr)) break;  // breaks the caller's loop
 * ```
 * @a [[nodiscard]]: the result is the whole point, so dropping it is a bug.
 */
[[nodiscard]] inline bool assert_check(
    bool cond,
    std::string_view cond_str,
    std::string_view info           = {},
    const std::source_location& loc = std::source_location::current())
{
    if (!cond) [[unlikely]]
    {
        assert_fail(cond_str, info, loc);
    }
    return cond;
}

/**
 * @brief Assert with bool return.
 * @note The macros below exist *only* to stringify the condition. They expand
 * to a plain call expression, so they never introduce a loop or a scope and the
 * enclosing control flow is left untouched.
 */
#define ilyth_check(...) ::ilyth::assert_check((__VA_ARGS__), #__VA_ARGS__)

#define ilyth_check2(cond, info) ::ilyth::assert_check((cond), #cond, (info))

/**
 * @brief More like standard assert.
 */
#define ilyth_assert(...) \
    ((void)::ilyth::assert_check((__VA_ARGS__), #__VA_ARGS__))

#define ilyth_assert2(cond, info) \
    ((void)::ilyth::assert_check((cond), #cond, (info)))

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

constexpr auto min_u8  = std::numeric_limits<u8>::min();
constexpr auto max_u8  = std::numeric_limits<u8>::max();
constexpr auto min_s8  = std::numeric_limits<s8>::min();
constexpr auto max_s8  = std::numeric_limits<s8>::max();
constexpr auto min_u16 = std::numeric_limits<u16>::min();
constexpr auto max_u16 = std::numeric_limits<u16>::max();
constexpr auto min_s16 = std::numeric_limits<s16>::min();
constexpr auto max_s16 = std::numeric_limits<s16>::max();
constexpr auto min_u32 = std::numeric_limits<u32>::min();
constexpr auto max_u32 = std::numeric_limits<u32>::max();
constexpr auto min_s32 = std::numeric_limits<s32>::min();
constexpr auto max_s32 = std::numeric_limits<s32>::max();
constexpr auto min_u64 = std::numeric_limits<u64>::min();
constexpr auto max_u64 = std::numeric_limits<u64>::max();
constexpr auto min_s64 = std::numeric_limits<s64>::min();
constexpr auto max_s64 = std::numeric_limits<s64>::max();

static_assert(min_u8 == 0);
static_assert(min_u16 == 0);
static_assert(min_u32 == 0);
static_assert(min_u64 == 0);

}  // namespace types

}  // namespace ilyth

#endif
