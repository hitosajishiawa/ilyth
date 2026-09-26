#ifndef ILYTH_SPINLOCK_H
#define ILYTH_SPINLOCK_H

#include "ilyth/platform.h"

#include <atomic>

#if ILYTH_PLATFORM_ARCH_ARM
    #include <arm_acle.h>
#endif

#if ILYTH_PLATFORM_LINUX_GLIBC
    #include <pthread.h>
#endif

namespace ilyth
{

/** Hint to the CPU that the current thread is spinning. */
inline auto cpu_relax() noexcept -> void
{
#if ILYTH_PLATFORM_ARCH_X86
    // builtin pause
    __builtin_ia32_pause();
#elif ILYTH_PLATFORM_ARCH_ARM
    __yield();
#else  // ILYTH_PLATFORM_ARCH_UNKNOWN
       // No pause/yield hint for other architectures
#endif
}

/**
 * A spinlock built only on `std::atomic_flag`.
 *
 * No platform or libc dependency, so it is available everywhere. This is what
 * `Spinlock` below names by default.
 *
 * Follows the test-and-test-and-set (TTAS) pattern: spin on a plain load while
 * the flag is held, and only retry the (expensive) read-modify-write once the
 * flag looks free.
 *
 * @warning Not a general-purpose mutex. It only pays off when the critical
 * section is a handful of instructions, the lock is never held across a
 * blocking call, and there are at most as many runnable threads as cores.
 * Otherwise a waiter burns a whole timeslice, and there is no fairness, no
 * timeout and no owner that a debugger can show you. Prefer `std::mutex`
 * unless you have measured that this wins on your workload.
 */
class AtomicSpinlock
{
private:
    std::atomic_flag mutex{};

public:
    AtomicSpinlock()  = default;
    ~AtomicSpinlock() = default;

    AtomicSpinlock(const AtomicSpinlock&)            = delete;
    AtomicSpinlock& operator=(const AtomicSpinlock&) = delete;

    void lock() noexcept
    {
        while (mutex.test_and_set(std::memory_order_acquire))
        {
            while (mutex.test(std::memory_order_relaxed))
            {
                cpu_relax();
            }
        }
    }
    [[nodiscard]] bool try_lock() noexcept
    {
        return !mutex.test_and_set(std::memory_order_acquire);
    }
    void unlock() noexcept { mutex.clear(std::memory_order_release); }
};

#if ILYTH_PLATFORM_LINUX_GLIBC

/**
 * A lock wrapping glibc's `pthread_spinlock_t`. Only available on Linux +
 * glibc.
 *
 * Offered as an alternative, **not** as the default: it lost to
 * `AtomicSpinlock` at every thread count measured by `tests/bench_lock.cpp`.
 * How much that costs depends on your critical section and your thread-to-core
 * ratio, so benchmark your own workload before deciding to use this one.
 *
 * @warning Same caveats as `AtomicSpinlock`. glibc additionally stops spinning
 * after ~1000 iterations and calls `sched_yield()`, so under contention this
 * behaves closer to a mutex than to a spinlock.
 */
class PthreadSpinlock
{
private:
    pthread_spinlock_t mutex{};

public:
    PthreadSpinlock() noexcept
    {
        pthread_spin_init(&mutex, PTHREAD_PROCESS_PRIVATE);
    }
    ~PthreadSpinlock() noexcept { pthread_spin_destroy(&mutex); }

    PthreadSpinlock(const PthreadSpinlock&)            = delete;
    PthreadSpinlock& operator=(const PthreadSpinlock&) = delete;

    void lock() noexcept { pthread_spin_lock(&mutex); }
    [[nodiscard]] bool try_lock() noexcept
    {
        return pthread_spin_trylock(&mutex) == 0;
    }
    void unlock() noexcept { pthread_spin_unlock(&mutex); }
};

#endif  // ILYTH_PLATFORM_LINUX_GLIBC

/**
 * The userspace spinlock this library uses by default.
 *
 * Unconditionally `AtomicSpinlock`, so the behavior does not change from one
 * platform to the next. Reach for `PthreadSpinlock` explicitly only after
 * measuring that it suits your workload better.
 *
 * @warning See `AtomicSpinlock`: a spinlock is not a general-purpose mutex.
 * Reach for `std::mutex` unless you have measured that this one wins.
 */
using Spinlock = AtomicSpinlock;

}  // namespace ilyth

#endif
