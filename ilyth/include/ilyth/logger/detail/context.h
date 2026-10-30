#ifndef ILYTH_LOGGER_DETAIL_CONTEXT_H
#define ILYTH_LOGGER_DETAIL_CONTEXT_H

#include <ilyth/types.h>

#include <chrono>

namespace ilyth::detail
{

/**
 * @return Fiber id; always 0 until a fiber runtime exists.
 */
[[nodiscard]] auto currentFiberId() noexcept -> types::u64;

/**
 * @return Time elapsed since the process launched.
 */
[[nodiscard]] auto elapsedSinceLaunch() noexcept
    -> std::chrono::steady_clock::duration;

}  // namespace ilyth::detail

#endif
