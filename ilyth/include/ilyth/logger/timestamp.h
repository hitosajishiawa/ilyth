#ifndef ILYTH_LOGGER_TIMESTAMP_H
#define ILYTH_LOGGER_TIMESTAMP_H

#include <ilyth/logger/detail/timestamp.h>

#include <algorithm>
#include <chrono>
#include <format>

namespace ilyth
{

/**
 * @brief Lightweight view that renders a time in local time, cached per second.
 *
 * Use it in a formatter policy:
 * ```
 * std::format_to(out, "{:.1} {}", ilyth::Timestamp{entry.timestamp}, ...);
 * ```
 * `"{:.N}"` selects N sub-second digits (0..9); plain `"{}"` uses
 * @ref detail::DefaultTimestampDigits. For a fully custom chrono layout,
 * format `entry.timestamp` directly instead -- that path is not cached.
 */
struct Timestamp
{
    std::chrono::system_clock::time_point value;
};

}  // namespace ilyth

template <>
struct std::formatter<ilyth::Timestamp, char>
{
    int digits = static_cast<int>(ilyth::detail::DefaultTimestampDigits);

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it  = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it == '.')
        {
            ++it;
            int parsed = 0;
            bool any   = false;
            while (it != end && *it >= '0' && *it <= '9')
            {
                parsed = parsed * 10 + (*it - '0');
                any    = true;
                ++it;
            }
            if (!any)
            {
                throw std::format_error("invalid Timestamp precision");
            }
            digits = parsed;
        }
        if (it != end && *it != '}')
        {
            throw std::format_error("invalid Timestamp format spec");
        }
        return it;
    }

    auto format(const ilyth::Timestamp& ts, std::format_context& ctx) const
    {
        char buffer[ilyth::detail::TimestampBufferSize];
        const auto len = ilyth::detail::format_timestamp(buffer, sizeof(buffer),
                                                         ts.value, digits);
        return std::copy_n(buffer, len, ctx.out());
    }
};

#endif
