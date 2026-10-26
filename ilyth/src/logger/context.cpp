#include <ilyth/logger/detail/context.h>

#include <ilyth/platform.h>

#include <chrono>

#if ILYTH_PLATFORM_LINUX
    #include <unistd.h>
    #include <cstdio>
    #include <cstring>
    #include <ctime>
    #include <sstream>
    #include <string>
#elif ILYTH_PLATFORM_WIN32
    #include "platform/win32.h"
#endif

namespace ilyth::detail
{

namespace
{

/**
 * @brief Age of this process at the time of the call.
 *
 * Returns a zero duration when it cannot be determined, so callers fall back to
 * "since first use".
 */
auto processAge() noexcept -> std::chrono::steady_clock::duration
{
#if ILYTH_PLATFORM_LINUX
    try
    {
        std::FILE* file = std::fopen("/proc/self/stat", "r");
        if (file == nullptr)
        {
            return {};
        }
        char buffer[4096];
        const char* line = std::fgets(buffer, sizeof(buffer), file);
        std::fclose(file);
        if (line == nullptr)
        {
            return {};
        }

        // `comm` may contain spaces and ')', so parse from the last ')'.
        const char* close = std::strrchr(buffer, ')');
        if (close == nullptr)
        {
            return {};
        }

        // Fields after comm: 3 = state (a char), 4..22 = numbers; starttime is
        // field 22, i.e. the 20th token after comm.
        std::istringstream fields{std::string(close + 1)};
        char state{};
        long long ppid{}, pgrp{}, session{}, tty_nr{}, tpgid{}, flags{};
        long long minflt{}, cminflt{}, majflt{}, cmajflt{}, utime{}, stime{};
        long long cutime{}, cstime{}, priority{}, nice{}, num_threads{};
        long long itrealvalue{};
        unsigned long long starttime{};
        fields >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >>
            flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime >>
            cutime >> cstime >> priority >> nice >> num_threads >>
            itrealvalue >> starttime;
        if (!fields)
        {
            return {};
        }

        const long ticks = ::sysconf(_SC_CLK_TCK);
        if (ticks <= 0)
        {
            return {};
        }

        timespec boot{};
        if (::clock_gettime(CLOCK_BOOTTIME, &boot) != 0)
        {
            return {};
        }

        const auto now_boot = std::chrono::seconds{boot.tv_sec} +
                              std::chrono::nanoseconds{boot.tv_nsec};
        // Keep the sub-second part: starttime is in ticks, so truncating to
        // whole seconds would inflate the age by up to ~1s.
        const auto start_boot =
            std::chrono::nanoseconds{static_cast<std::chrono::nanoseconds::rep>(
                static_cast<long double>(starttime) * 1'000'000'000.0L /
                static_cast<long double>(ticks))};
        if (now_boot < start_boot)
        {
            return {};
        }
        return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            now_boot - start_boot);
    }
    catch (...)
    {
        return {};
    }
#elif ILYTH_PLATFORM_WIN32
    FILETIME creation{};
    FILETIME exit{};
    FILETIME kernel{};
    FILETIME user{};
    if (::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel,
                          &user) == 0)
    {
        return {};
    }

    FILETIME now{};
    ::GetSystemTimeAsFileTime(&now);

    ULARGE_INTEGER created{};
    created.LowPart  = creation.dwLowDateTime;
    created.HighPart = creation.dwHighDateTime;
    ULARGE_INTEGER current{};
    current.LowPart  = now.dwLowDateTime;
    current.HighPart = now.dwHighDateTime;
    if (current.QuadPart <= created.QuadPart)
    {
        return {};
    }

    // FILETIME counts 100-ns intervals.
    return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::nanoseconds{static_cast<std::chrono::nanoseconds::rep>(
            (current.QuadPart - created.QuadPart) * 100)});
#else
    return {};
#endif
}

}  // namespace

auto currentFiberId() noexcept -> types::u64
{
    return 0;
}

auto elapsedSinceLaunch() noexcept -> std::chrono::steady_clock::duration
{
    // Computed once. The reference is the OS-reported process start.
    // If unavailable, fall back to "since first use".
    static const auto launch = [] {
        const auto now = std::chrono::steady_clock::now();
        const auto age = processAge();
        return age > std::chrono::steady_clock::duration::zero() ? now - age
                                                                 : now;
    }();
    return std::chrono::steady_clock::now() - launch;
}

}  // namespace ilyth::detail
