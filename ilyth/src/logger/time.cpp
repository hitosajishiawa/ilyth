#include <ilyth/logger/detail/time.h>

#include <cstddef>
#include <cstring>
#include <ctime>

namespace ilyth::detail
{

auto format_timestamp(char* out,
                      std::size_t capacity,
                      std::chrono::system_clock::time_point tp,
                      int digits) noexcept -> std::size_t
{
    // the expensive part: calendar decomposition, cached per second
    thread_local std::chrono::sys_seconds cached_second{};
    thread_local std::size_t cached_len           = 0;
    thread_local char cached[TimestampBufferSize] = {};

    const auto whole = std::chrono::floor<std::chrono::seconds>(tp);
    if (cached_len == 0 || whole != cached_second)
    {
        const std::time_t secs = std::chrono::system_clock::to_time_t(whole);
        std::tm local{};
#if ILYTH_PLATFORM_WIN32
        ::localtime_s(&local, &secs);
#else
        ::localtime_r(&secs, &local);
#endif
        cached_len =
            std::strftime(cached, sizeof(cached), "%Y-%m-%d %H:%M:%S", &local);
        cached_second = whole;
    }

    // copy the cached prefix, then append the sub-second tail
    std::size_t written = 0;
    const auto emit     = [&](const char* src, std::size_t len) {
        const std::size_t room = capacity > written ? capacity - written : 0;
        const std::size_t take = len < room ? len : room;
        if (take != 0)
        {
            std::memcpy(out + written, src, take);
        }
        written += len;  // logical length, even when truncated
    };

    emit(cached, cached_len);

    // `tp - whole` is below one second, so it *is* the sub-second remainder.
    const auto sub =
        std::chrono::duration_cast<std::chrono::nanoseconds>(tp - whole)
            .count();
    char tail[MaxFractionDigits + 1];
    emit(tail, render_fraction(tail, sub, digits));

    return written;
}

}  // namespace ilyth::detail
