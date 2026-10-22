#ifndef ILYTH_LOGGER_DETAIL_CONTEXT_H
#define ILYTH_LOGGER_DETAIL_CONTEXT_H

#include <ilyth/types.h>

#include <chrono>
#include <string_view>

namespace ilyth::detail
{

/// @return OS thread id (`gettid` on Linux), or a hash fallback elsewhere.
[[nodiscard]] auto current_thread_id() noexcept -> types::u64;

/// @return Current thread name as a thread-local view; empty if unsupported.
[[nodiscard]] auto current_thread_name() noexcept -> std::string_view;

/// @return Fiber id; always 0 until a fiber runtime exists.
[[nodiscard]] auto current_fiber_id() noexcept -> types::u64;

/// @return Time elapsed since first use (process-start proxy).
[[nodiscard]] auto elapsed_since_start() noexcept
    -> std::chrono::steady_clock::duration;

}  // namespace ilyth::detail

#endif
