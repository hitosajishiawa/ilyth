#include <ilyth/logger/detail/context.h>

#include <ilyth/platform.h>

#include <chrono>

#if ILYTH_PLATFORM_LINUX_GLIBC
    #include <pthread.h>
    #include <unistd.h>
#else
    #include <functional>
    #include <thread>
#endif

namespace ilyth::detail
{

auto current_thread_id() noexcept -> types::u64
{
#if ILYTH_PLATFORM_LINUX_GLIBC
    return static_cast<types::u64>(::gettid());
#else
    return static_cast<types::u64>(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));
#endif
}

auto current_thread_name() noexcept -> std::string_view
{
#if ILYTH_PLATFORM_LINUX_GLIBC
    // Cached once per thread: the kernel name is fixed at thread creation.
    thread_local char name[16] = {};
    thread_local bool loaded   = false;
    if (!loaded)
    {
        ::pthread_getname_np(::pthread_self(), name, sizeof(name));
        loaded = true;
    }
    return name;
#else
    return {};
#endif
}

auto current_fiber_id() noexcept -> types::u64
{
    return 0;
}

auto elapsed_since_start() noexcept -> std::chrono::steady_clock::duration
{
    static const auto start = std::chrono::steady_clock::now();
    return std::chrono::steady_clock::now() - start;
}

}  // namespace ilyth::detail
