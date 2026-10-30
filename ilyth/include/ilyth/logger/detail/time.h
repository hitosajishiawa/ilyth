#ifndef ILYTH_LOGGER_DETAIL_TIME_H
#define ILYTH_LOGGER_DETAIL_TIME_H

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <utility>

namespace ilyth::detail
{

/// Sub-second digits rendered when a format spec gives no precision.
inline constexpr int DefaultTimeFractionDigits = 3;

/// Hard resolution bound: a clock cannot report sub-nanoseconds.
/// It makes no sense to modify the value.
inline constexpr int MaxFractionDigits = 9;

/// Nanoseconds in one second, i.e. `10^9`.
inline constexpr std::int64_t NanosPerSecond = 1'000'000'000;

/// Enough for "YYYY-MM-DD HH:MM:SS" plus up to 9 sub-second digits and a dot.
inline constexpr std::size_t TimestampBufferSize = 32;

/// Enough for a 9-digit day field, "d ", "HH:MM:SS", ".", 9 digits, NUL.
inline constexpr std::size_t ElapsedBufferSize = 32;

/// `10^0 .. 10^MaxFractionDigits`, in ascending order.
inline constexpr auto FractionScale = []() {
    std::array<std::int64_t, MaxFractionDigits + 1> table{};
    std::int64_t value = 1;
    for (auto& entry : table)
    {
        entry = value;
        value *= 10;
    }
    return table;
}();

static_assert(FractionScale.front() == 1);
static_assert(FractionScale.back() == NanosPerSecond);

/// Result of parsing a `".N"` sub-second precision.
struct FractionSpec
{
    int digits;
    std::format_parse_context::iterator end;
};

/**
 * @brief Parse the optional `".N"` sub-second precision of a format spec.
 *
 * Consumes `".N"` if present and requires the closing `}`.
 * `DefaultTimestampDigits` applies when no precision is given.
 *
 * @throws std::format_error when the spec is malformed, or when `N` exceeds
 * `MaxFractionDigits`.
 */
constexpr auto parse_fraction_spec(std::format_parse_context& ctx)
    -> FractionSpec
{
    auto it        = ctx.begin();
    const auto end = ctx.end();
    int digits     = DefaultTimeFractionDigits;

    if (it != end && *it == '.')
    {
        ++it;
        int parsed = 0;
        bool any   = false;
        while (it != end && *it >= '0' && *it <= '9')
        {
            // Freeze the accumulator once it is out of range: the check below
            // rejects it, and freezing keeps a long spec from overflowing.
            if (parsed <= MaxFractionDigits)
            {
                parsed = parsed * 10 + (*it - '0');
            }
            any = true;
            ++it;
        }
        if (!any)
        {
            throw std::format_error("missing sub-second precision");
        }
        if (parsed > MaxFractionDigits)
        {
            throw std::format_error("sub-second precision out of range");
        }
        digits = parsed;
    }

    if (it != end && *it != '}')
    {
        throw std::format_error("invalid format spec");
    }
    return FractionSpec{digits, it};
}

/// Split a nanosecond count into whole seconds and the sub-second remainder.
constexpr auto split_nanos(std::int64_t nanos) noexcept
    -> std::pair<std::int64_t, std::int64_t>
{
    return {nanos / NanosPerSecond, nanos % NanosPerSecond};
}

/**
 * @brief Render the sub-second tail of a time value: `".fff"`, @p digits long.
 *
 * Both time views share this tail, so precision -- and the fact that it is
 * truncated rather than rounded -- behaves the same whichever one is printed.
 *
 * @param out       Destination with room for `digits + 1` bytes.
 * @param subNanos  Sub-second remainder, in `[0, NanosPerSecond)` .
 * @param digits    Digits to emit.
 * @pre @p digits is in `[0, MaxFractionDigits]` . Parsing a format spec already
 * guarantees it; violating it is undefined behaviour.
 * @return Bytes written: 0 when @p digits is 0, else @p digits + 1.
 */
constexpr auto render_fraction(char* out,
                               std::int64_t subNanos,
                               int digits) noexcept -> std::size_t
{
    if (digits <= 0)
    {
        return 0;
    }
    out[0] = '.';

    auto fraction = subNanos / FractionScale[MaxFractionDigits - digits];
    for (auto i = digits; i >= 1; --i)
    {
        out[i] = static_cast<char>('0' + fraction % 10);
        fraction /= 10;
    }
    return static_cast<std::size_t>(digits) + 1;
}

/**
 * @brief Render @p tp as local time: `"YYYY-MM-DD HH:MM:SS"` plus @p digits
 * sub-second digits.
 *
 * Only the calendar decomposition is expensive, and it is invariant within a
 * second, so it is cached per (thread, second). The sub-second part is cheap
 * and rendered every call, which keeps the caller's precision choice free.
 *
 * @param out       Destination; may be smaller than the needed size.
 * @param capacity  Writable bytes at @p out .
 * @param digits    Sub-second digits to append.
 * @pre @p digits is in `[0, MaxFractionDigits]`. Parsing a format spec already
 * guarantees it; violating it is undefined behaviour.
 * @return Needed length (two-pass contract: may exceed @p capacity ).
 */
auto format_timestamp(char* out,
                      std::size_t capacity,
                      std::chrono::system_clock::time_point tp,
                      int digits) noexcept -> std::size_t;

}  // namespace ilyth::detail

#endif
