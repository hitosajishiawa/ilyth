#ifndef ILYTH_LOGGER_DETAIL_BUFFER_H
#define ILYTH_LOGGER_DETAIL_BUFFER_H

#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace ilyth::detail
{

/**
 * @brief Contiguous scratch buffer: stack array first, whole-buffer heap
 * fallback on overflow.
 */
template <std::size_t InlineCapacity>
class LoggerBuffer
{
public:
    /// Contiguous inline region. Precondition: the buffer is empty.
    [[nodiscard]] auto writable() noexcept -> std::span<char>
    {
        return {mInlineStorage.data(), InlineCapacity};
    }

    /// Publish `n` bytes produced into @ref writable (`n <= InlineCapacity`).
    auto commit(std::size_t n) noexcept -> void { mLength = n; }

    /// Drop the content, hand back exactly `n` heap bytes (overflow path).
    [[nodiscard]] auto allocate(std::size_t n) -> std::span<char>
    {
        mHeapStorage.resize(n);
        mLength = n;
        return {mHeapStorage.data(), n};
    }

    /// Direct access to the underlying contiguous storage.
    [[nodiscard]] auto data() const noexcept -> const char*
    {
        return mHeapStorage.empty() ? mInlineStorage.data()
                                    : mHeapStorage.data();
    }

    /// Get the length of written chars.
    [[nodiscard]] auto size() const noexcept -> std::size_t { return mLength; }

    /// Get a sv of written chars in the underlying storage.
    [[nodiscard]] auto view() const noexcept -> std::string_view
    {
        return {data(), size()};
    }

private:
    /**
     * @note Intentionally left uninitialized: avoid the zero-filling on every
     * log call. This is safe because no byte is ever read before it is written.
     */
    std::array<char, InlineCapacity> mInlineStorage;
    std::vector<char> mHeapStorage;
    std::size_t mLength = 0;
};

/**
 * @brief Two-pass fill: produce into the inline region, retry on the heap only
 * if it does not fit.
 *
 * @p produce takes the destination span and returns the needed length.
 */
template <std::size_t InlineCapacity, class Producer>
auto formatTwoPass(LoggerBuffer<InlineCapacity>& buffer, Producer&& produce)
    -> void
{
    const auto room   = buffer.writable();
    const auto needed = produce(room);
    if (needed <= room.size())
    {
        buffer.commit(needed);
        return;
    }
    (void)produce(buffer.allocate(needed));
}

}  // namespace ilyth::detail

#endif
