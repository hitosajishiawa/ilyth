#ifndef ILYTH_LOGGER_DETAIL_TIMESTAMP_H
#define ILYTH_LOGGER_DETAIL_TIMESTAMP_H

#include <chrono>
#include <cstddef>

namespace ilyth::detail
{

/// Enough for "YYYY-MM-DD HH:MM:SS" plus up to 9 sub-second digits and a dot.
inline constexpr std::size_t TimestampBufferSize = 40;

/// Sub-second digits rendered when a format spec gives no precision.
inline constexpr std::size_t DefaultTimestampDigits = 3;

/**
 * @brief Render @p tp as local time: `"YYYY-MM-DD HH:MM:SS"` plus @p digits
 * sub-second digits (0..9, clamped).
 *
 * Only the calendar decomposition is expensive, and it is invariant within a
 * second, so it is cached per (thread, second). The sub-second part is cheap
 * and rendered every call, which keeps the caller's precision choice free.
 *
 * @param out       Destination; may be smaller than the needed size.
 * @param capacity  Writable bytes at @p out .
 * @return Needed length (two-pass contract: may exceed @p capacity ).
 */
auto format_timestamp(char* out,
                      std::size_t capacity,
                      std::chrono::system_clock::time_point tp,
                      int digits) noexcept -> std::size_t;

}  // namespace ilyth::detail

#endif
