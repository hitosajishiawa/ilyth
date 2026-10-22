#include <ilyth/logger/detail/timestamp.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>

namespace ilyth::detail
{

namespace
{

/// Hard resolution bound. It makes no sense to modify the value.
constexpr int MaxDigits = 9;

/// `10^0 .. 10^MaxDigits`, in ascending order.
constexpr auto Pow10 = []() {
    std::array<std::int64_t, MaxDigits + 1> table{};
    std::int64_t value = 1;
    for (auto& entry : table)
    {
        entry = value;
        value *= 10;
    }
    return table;
}();

static_assert(Pow10.front() == 1);
static_assert(Pow10.back() == std::chrono::nanoseconds::period::den);

}  // namespace

auto format_timestamp(char* out,
                      std::size_t capacity,
                      std::chrono::system_clock::time_point tp,
                      int digits) noexcept -> std::size_t
{
    // Clamp to the sub-second digits we can render.
    digits = std::clamp(digits, 0, MaxDigits);

    // the expensive part: calendar decomposition, cached per second
    thread_local std::chrono::sys_seconds cached_second{};
    thread_local std::size_t cached_len           = 0;
    thread_local char cached[TimestampBufferSize] = {};

    const auto whole = std::chrono::floor<std::chrono::seconds>(tp);
    if (cached_len == 0 || whole != cached_second)
    {
        const std::time_t secs = std::chrono::system_clock::to_time_t(whole);
        std::tm local{};
#if defined(_WIN32)
        ::localtime_s(&local, &secs);
#else
        ::localtime_r(&secs, &local);
#endif
        cached_len =
            std::strftime(cached, sizeof(cached), "%Y-%m-%d %H:%M:%S", &local);
        cached_second = whole;
    }

    // copy the cached prefix, then append the sub-second digits
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

    if (digits > 0)
    {
        emit(".", 1);

        const auto sub =
            std::chrono::duration_cast<std::chrono::nanoseconds>(tp - whole)
                .count();
        std::int64_t fraction = sub / Pow10[MaxDigits - digits];

        char buffer[MaxDigits];
        for (int i = digits - 1; i >= 0; --i)
        {
            buffer[i] = static_cast<char>('0' + fraction % 10);
            fraction /= 10;
        }
        emit(buffer, static_cast<std::size_t>(digits));
    }

    return written;
}

}  // namespace ilyth::detail
