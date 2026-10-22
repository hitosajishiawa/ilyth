// Microbenchmark: logger formatting cost (formatting only, no I/O).
//
// Not pass/fail. Prints ns/op so ilyth's std::format path can be compared
// against pans' pre-parsed formatter and against the localtime+strftime cache.
#include <ilyth/logger/detail/buffer.h>
#include <ilyth/logger/entry.h>
#include <ilyth/logger/formatter.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <format>
#include <span>

namespace
{
using Clock = std::chrono::steady_clock;

template <class Fn>
double bench(int n, Fn&& fn)
{
    const auto t0 = Clock::now();
    for (int i = 0; i < n; ++i)
    {
        fn(i);
    }
    const auto t1 = Clock::now();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}
}  // namespace

int main(int argc, char** argv)
{
    const int n     = argc > 1 ? std::atoi(argv[1]) : 1000000;
    const auto base = std::chrono::system_clock::now();

    ilyth::LogEntry entry{};
    entry.level      = ilyth::LogLevel::Level::INFO;
    entry.loggerName = "bench/logger";
    entry.message    = "hello 42 world x";
    entry.timestamp  = base;
    entry.elapsed    = std::chrono::milliseconds(1234);
    entry.threadID   = 12345;
    entry.fiberID    = 0;
    entry.threadName = "main";

    std::uint64_t sink = 0;

    // Full line, same second (what a datetime cache would be able to reuse).
    const double full_fixed = bench(n, [&](int) {
        std::array<char, 512> buf;
        sink += ilyth::defaultFormatter(entry, std::span<char>(buf));
    });

    // Full line, second advances every call.
    const double full_varying = bench(n, [&](int i) {
        ilyth::LogEntry e = entry;
        e.timestamp       = base + std::chrono::seconds(i);
        std::array<char, 512> buf;
        sink += ilyth::defaultFormatter(e, std::span<char>(buf));
    });

    // Full line with the timestamp already rendered (what a cache gives).
    constexpr std::string_view cached_ts_text = "2026-10-22 12:34:56";
    const double full_cached_ts               = bench(n, [&](int) {
        std::array<char, 512> buf;
        sink += static_cast<std::uint64_t>(
            std::format_to_n(buf.data(),
                             static_cast<std::ptrdiff_t>(buf.size()),
                             "{} [{}] {}:{} {}\n", cached_ts_text, entry.level,
                             entry.location.file_name(), entry.location.line(),
                             entry.message)
                .size);
    });

    // Integer formatting + format-string parsing only (no chrono).
    const double int_only = bench(n, [&](int i) {
        std::array<char, 32> buf;
        sink += static_cast<std::uint64_t>(
            std::format_to_n(buf.data(),
                             static_cast<std::ptrdiff_t>(buf.size()), "{}", i)
                .size);
    });

    // std::format chrono timestamp only (ilyth's path).
    const double fmt_ts = bench(n, [&](int) {
        std::array<char, 32> buf;
        sink += static_cast<std::uint64_t>(
            std::format_to_n(buf.data(),
                             static_cast<std::ptrdiff_t>(buf.size()),
                             "{:%Y-%m-%d %H:%M:%S}", entry.timestamp)
                .size);
    });

    // localtime_r + strftime, no cache (pans' unoptimized path).
    const double local_ts = bench(n, [&](int i) {
        const auto tp  = base + std::chrono::seconds(i);
        const auto sec = static_cast<std::time_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                tp.time_since_epoch())
                .count());
        std::tm tm{};
        localtime_r(&sec, &tm);
        char buf[32];
        sink += std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    });

    // localtime_r + strftime, cached by second (pans' optimized path).
    static thread_local std::time_t last_second = 0;
    static thread_local char cached[32]         = {};
    const double cached_ts                      = bench(n, [&](int) {
        const auto sec = static_cast<std::time_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                entry.timestamp.time_since_epoch())
                .count());
        if (sec != last_second)
        {
            std::tm tm{};
            localtime_r(&sec, &tm);
            std::strftime(cached, sizeof(cached), "%Y-%m-%d %H:%M:%S", &tm);
            last_second = sec;
        }
        sink += std::strlen(cached);
    });

    std::printf(
        "ilyth defaultFormatter full line  same-second  : %8.1f ns/op\n",
        full_fixed);
    std::printf(
        "ilyth defaultFormatter full line  varying-sec  : %8.1f ns/op\n",
        full_varying);
    std::printf(
        "ilyth full line w/ cached timestamp string      : %8.1f ns/op\n",
        full_cached_ts);
    std::printf(
        "std::format chrono timestamp only               : %8.1f ns/op\n",
        fmt_ts);
    std::printf(
        "std::format int only (parse + to_chars)         : %8.1f ns/op\n",
        int_only);
    std::printf(
        "localtime_r+strftime (no cache)                 : %8.1f ns/op\n",
        local_ts);
    std::printf(
        "localtime_r+strftime (cached by second)         : %8.1f ns/op\n",
        cached_ts);
    std::printf("(sink=%llu)\n", static_cast<unsigned long long>(sink));
    return 0;
}
