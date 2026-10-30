#ifndef ILYTH_LOGGER_TIME_H
#define ILYTH_LOGGER_TIME_H

#include <ilyth/logger/detail/time.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>

namespace ilyth
{

/**
 * @brief Lightweight view that renders a time in local time, cached per second.
 *
 * Use it in a formatter policy:
 * ```
 * std::format_to(out, "{:.1} {}", entry.timestamp, ...);
 * ```
 * where `"{:.N}"` selects `N` sub-second digits and will be rejected with
 * `std::format_error` if exceeding `detail::MaxFractionDigits`.
 * Default `"{}"` uses @ref detail::DefaultTimestampDigits .
 *
 * For a fully custom chrono layout, format `entry.timestamp.value` directly
 * instead -- that path is not cached.
 */
struct Timestamp
{
    std::chrono::system_clock::time_point value;
};

/**
 * @brief Lightweight view that renders a duration as `[<days>d ]HH:MM:SS.fff`.
 *
 * The day field appears only once one has elapsed, so a short-lived process
 * reads `00:00:08.123` while a server up for 105 days reads `105d 04:31:07.123`
 * (hours wrap at 24 as soon as the day field is present). Days are never
 * zero-padded.
 *
 * Use it in a formatter policy:
 * ```
 * std::format_to(out, "{:.1} {}", entry.elapsed, ...);
 * ```
 * `"{:.N}"` means the same as for @ref Timestamp .
 */
struct Elapsed
{
    std::chrono::steady_clock::duration value;
};

}  // namespace ilyth

namespace ilyth::detail
{

/**
 * @brief Shared `std::formatter` base for the two time views.
 *
 * Both accept exactly one spec, the `"{:.N}"` sub-second precision, so parsing
 * it once here leaves each specialization with just its `format` body.
 */
struct FractionDigitsFormatter
{
    /// Defaulted so the member is never read indeterminate; `parse` overwrites
    /// it whenever the spec carries a precision.
    int mDigits = DefaultTimeFractionDigits;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        const auto spec = parse_fraction_spec(ctx);
        mDigits         = spec.digits;
        return spec.end;
    }
};

}  // namespace ilyth::detail

template <>
struct std::formatter<ilyth::Timestamp, char>
    : ilyth::detail::FractionDigitsFormatter
{
    auto format(const ilyth::Timestamp& ts, std::format_context& ctx) const
    {
        char buffer[ilyth::detail::TimestampBufferSize];
        const auto len = ilyth::detail::format_timestamp(buffer, sizeof(buffer),
                                                         ts.value, mDigits);
        return std::copy_n(buffer, len, ctx.out());
    }
};

template <>
struct std::formatter<ilyth::Elapsed, char>
    : ilyth::detail::FractionDigitsFormatter
{
    auto format(const ilyth::Elapsed& elapsed, std::format_context& ctx) const
    {
        char buffer[ilyth::detail::ElapsedBufferSize];
        std::size_t len = 0;

        auto nanos =
            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed.value)
                .count();
        nanos = std::max<std::int64_t>(nanos, 0);  // never negative in practice

        constexpr std::int64_t SecondsPerMinute = 60;
        constexpr std::int64_t SecondsPerHour   = 60 * SecondsPerMinute;
        constexpr std::int64_t SecondsPerDay    = 24 * SecondsPerHour;

        const auto [totalSeconds, subNanos] = ilyth::detail::split_nanos(nanos);

        // Days are shown only once one has elapsed, so the usual short-lived
        // case keeps the compact `HH:MM:SS.fff` shape.
        if (const auto days = totalSeconds / SecondsPerDay; days > 0)
        {
            char dayDigits[20]   = {};
            std::size_t dayCount = 0;
            auto rest            = days;
            do
            {
                dayDigits[dayCount++] = static_cast<char>('0' + rest % 10);
                rest /= 10;
            } while (rest != 0);
            while (dayCount > 0)
            {
                buffer[len++] = dayDigits[--dayCount];
            }
            buffer[len++] = 'd';
            buffer[len++] = ' ';
        }

        const auto emit_field = [&](std::int64_t value) {
            buffer[len++] = static_cast<char>('0' + value / 10);
            buffer[len++] = static_cast<char>('0' + value % 10);
        };
        emit_field((totalSeconds / SecondsPerHour) % 24);
        buffer[len++] = ':';
        emit_field((totalSeconds / SecondsPerMinute) % 60);
        buffer[len++] = ':';
        emit_field(totalSeconds % 60);

        len += ilyth::detail::render_fraction(buffer + len, subNanos, mDigits);
        return std::copy_n(buffer, len, ctx.out());
    }
};

#endif
