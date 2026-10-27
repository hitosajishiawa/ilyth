#ifndef ILYTH_UTILS_THREAD_H
#define ILYTH_UTILS_THREAD_H

#include <ilyth/export.h>
#include <ilyth/types.h>

#include <string_view>

namespace ilyth
{

/**
 * @brief OS thread id: the kernel TID on Linux, the system thread id on
 * Windows, or a hash of `std::thread::id` where neither is available.
 */
[[nodiscard]] ILYTH_API auto currentThreadId() noexcept -> types::u64;

/**
 * @brief Current thread name as a thread-local view; empty if unsupported.
 *
 * Cached per thread and refreshed by @ref setCurrentThreadName ; a name set
 * through the OS directly is not observed until then.
 */
[[nodiscard]] ILYTH_API auto currentThreadName() noexcept -> std::string_view;

/**
 * @brief Best-effort rename of the current thread (visible to tools such as
 * `ps` and to @ref currentThreadName).
 *
 * @note Linux keeps at most 15 bytes; longer names are truncated.
 */
ILYTH_API void setCurrentThreadName(std::string_view name) noexcept;

}  // namespace ilyth

#endif
