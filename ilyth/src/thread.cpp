#include <ilyth/thread.h>

#include <ilyth/platform.h>

#if ILYTH_PLATFORM_LINUX
    #include <sys/prctl.h>
    #include <sys/syscall.h>
    #include <unistd.h>
    #include <algorithm>
    #include <cstring>
#elif ILYTH_PLATFORM_WIN32
    #include "platform/win32.h"

    #include <string>
#else
    #include <functional>
    #include <thread>
#endif

namespace ilyth
{

namespace
{
#if ILYTH_PLATFORM_LINUX
// Linux keeps at most 15 bytes of thread name plus the terminating NUL.
inline constexpr std::size_t ThreadNameCapacity = 16;

thread_local char t_threadName[ThreadNameCapacity] = {};
thread_local bool t_threadNameLoaded               = false;
#elif ILYTH_PLATFORM_WIN32
// Windows descriptions are UTF-16; cache the UTF-8 form we expose.
thread_local std::string t_threadName;
thread_local bool t_threadNameLoaded = false;
#endif
}  // namespace

auto currentThreadId() noexcept -> types::u64
{
#if ILYTH_PLATFORM_LINUX
    // Raw syscall: no libc dependency (works with glibc and musl alike).
    static thread_local const types::u64 id =
        static_cast<types::u64>(::syscall(SYS_gettid));
    return id;
#elif ILYTH_PLATFORM_WIN32
    static thread_local const types::u64 id =
        static_cast<types::u64>(::GetCurrentThreadId());
    return id;
#else
    static thread_local const types::u64 id = static_cast<types::u64>(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));
    return id;
#endif
}

auto currentThreadName() noexcept -> std::string_view
{
#if ILYTH_PLATFORM_LINUX
    if (!t_threadNameLoaded)
    {
        (void)::prctl(PR_GET_NAME, t_threadName);
        t_threadNameLoaded = true;
    }
    return t_threadName;
#elif ILYTH_PLATFORM_WIN32
    if (!t_threadNameLoaded)
    {
        t_threadNameLoaded = true;
        PWSTR raw          = nullptr;
        if (SUCCEEDED(::GetThreadDescription(::GetCurrentThread(), &raw)) &&
            raw != nullptr)
        {
            try
            {
                const int bytes = ::WideCharToMultiByte(
                    CP_UTF8, 0, raw, -1, nullptr, 0, nullptr, nullptr);
                if (bytes > 1)
                {
                    t_threadName.resize(static_cast<std::size_t>(bytes - 1));
                    (void)::WideCharToMultiByte(CP_UTF8, 0, raw, -1,
                                                t_threadName.data(), bytes,
                                                nullptr, nullptr);
                }
            }
            catch (...)
            {
                t_threadName.clear();
            }
            (void)::LocalFree(raw);
        }
    }
    return t_threadName;
#else
    return {};
#endif
}

void setCurrentThreadName(std::string_view name) noexcept
{
#if ILYTH_PLATFORM_LINUX
    const std::size_t size = std::min(name.size(), ThreadNameCapacity - 1);
    std::memset(t_threadName, 0, sizeof(t_threadName));
    if (size != 0)
    {
        std::memcpy(t_threadName, name.data(), size);
    }
    (void)::prctl(PR_SET_NAME, t_threadName);
    t_threadNameLoaded = true;
#elif ILYTH_PLATFORM_WIN32
    try
    {
        if (name.empty())
        {
            (void)::SetThreadDescription(::GetCurrentThread(), L"");
            t_threadName.clear();
        }
        else
        {
            const int wide_count = ::MultiByteToWideChar(
                CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr,
                0);
            if (wide_count > 0)
            {
                std::wstring wide(static_cast<std::size_t>(wide_count), L'\0');
                (void)::MultiByteToWideChar(CP_UTF8, 0, name.data(),
                                            static_cast<int>(name.size()),
                                            wide.data(), wide_count);
                (void)::SetThreadDescription(::GetCurrentThread(),
                                             wide.c_str());
            }
            t_threadName.assign(name.data(), name.size());
        }
        t_threadNameLoaded = true;
    }
    catch (...)
    {
        // Allocation failure: keep the previous cache.
    }
#else
    (void)name;
#endif
}

}  // namespace ilyth
