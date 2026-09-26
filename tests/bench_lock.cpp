#include <ilyth/assert.h>
#include <ilyth/types.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <latch>
#include <mutex>
#include <print>
#include <ranges>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <vector>

using namespace ilyth::types;
using ns_t = std::chrono::nanoseconds;

constexpr auto Runs         = 7;
constexpr auto threadCounts = std::array{1, 2, 4, 8, 12};
constexpr auto target       = u64{10'000'000};

auto counter       = u64{};
auto atomicCounter = std::atomic<u64>{0};
auto mutex         = std::mutex{};
auto shatex        = std::shared_mutex{};  // short for sharedMutex

// A benchmark entry: how to reset the shared state, and what one thread does.
// Each operation is an independent chunk of work, so threads no longer need to
// be told how many of them there are.
struct Benchmark
{
    std::string_view name;
    void (*reset)();
    void (*operation)(u64);
};

// Entries to be benchmarked
constexpr auto benchmarks = std::array{
    Benchmark{"mutex", [] { counter = 0; },
              [](u64 operations) {
    for (const auto _ : std::views::iota(min_u64, operations))
    {
        auto guard = std::lock_guard{mutex};
        ++counter;
    }
}},
    Benchmark{"shared_mutex", [] { counter = 0; },
              [](u64 operations) {
    for (const auto _ : std::views::iota(min_u64, operations))
    {
        auto guard = std::lock_guard{shatex};
        ++counter;
    }
}},
    Benchmark{"atomic",
              [] { atomicCounter.store(0, std::memory_order_relaxed); },
              [](u64 operations) {
    for (const auto _ : std::views::iota(min_u64, operations))
    {
        atomicCounter.fetch_add(1, std::memory_order_relaxed);
    }
}},
    Benchmark{"atomic CAS",
              [] { atomicCounter.store(0, std::memory_order_relaxed); },
              [](u64 operations) {
    for (const auto _ : std::views::iota(min_u64, operations))
    {
        auto expected = atomicCounter.load(std::memory_order_relaxed);
        while (!atomicCounter.compare_exchange_weak(expected, expected + 1,
                                                    std::memory_order_relaxed,
                                                    std::memory_order_relaxed))
        {
        }
    }
}},
};

/** What a thread should do. */
auto assignWork = [](std::size_t thread_index, std::size_t thread_count) {
    const auto base      = target / thread_count;
    const auto remainder = target % thread_count;
    return base + (thread_index < remainder ? 1 : 0);
};

/** Create threads and run. */
auto runThreads = [](std::size_t thread_count, auto&& operation) -> ns_t {
    auto tc     = static_cast<std::ptrdiff_t>(thread_count);
    auto ready  = std::latch{tc};
    auto gate   = std::latch{1};
    auto finish = std::latch{tc};

    auto threads = std::vector<std::thread>{};
    threads.reserve(thread_count);
    for (const auto i : std::views::iota(min_u64, thread_count))
    {
        threads.emplace_back([&, i]() {
            ready.count_down();
            gate.wait();
            operation(assignWork(i, thread_count));
            finish.count_down();
        });
    }

    ready.wait();
    const auto begin = std::chrono::steady_clock::now();
    gate.count_down();
    finish.wait();
    const auto end = std::chrono::steady_clock::now();

    for (auto& th : threads)
    {
        th.join();
    }
    return std::chrono::duration_cast<ns_t>(end - begin);
};

/**
 * Repeat running several times and return the mean duration of one run.
 * `target` operations are spread across `thread_count` threads per run.
 */
const auto runBench = [](std::size_t thread_count,
                         const Benchmark& bm) -> double {
    auto total_ns = ns_t::rep{};  // The type is unspecified in standard,
                                  // depending on implementation
    for (int run = 0; run < Runs; ++run)
    {
        bm.reset();
        total_ns += runThreads(thread_count, bm.operation).count();
    }
    return static_cast<double>(total_ns) / Runs;
};

int main()
{
    std::print("operations: {}, average of {} runs\n\n", target, Runs);
    for (const auto tc : threadCounts)
    {
        std::print("threads: {}\n", tc);
        for (const auto& bm : benchmarks)
        {
            const auto avg_ns    = runBench(tc, bm);
            const auto ns_per_op = avg_ns / static_cast<double>(target);
            std::print("{:<14} {:.2f} ns total, {:.2f} ns/op\n", bm.name,
                       avg_ns, ns_per_op);
        }
        std::println();
    }
}
