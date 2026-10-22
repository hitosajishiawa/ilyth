#ifndef ILYTH_LOGGER_DETAIL_BUFFER_H
#define ILYTH_LOGGER_DETAIL_BUFFER_H

#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace ilyth::detail
{

inline constexpr auto LogMessageInlineCapacity     = std::size_t{128};
inline constexpr auto FormattedEntryInlineCapacity = std::size_t{256};

/**
 * @brief Contiguous scratch buffer: stack array first, whole-buffer heap
 * fallback on overflow.
 *
 * Deliberately not segmented (stack head + heap tail). A single
 * `std::format_to_n` call needs one contiguous destination, so a partial spill
 * buys nothing: on overflow the whole content is produced on the heap instead.
 *
 * @note No `value_type` / `push_back(char)` on purpose -- those exist only to
 * satisfy `std::back_inserter`, which feeds the buffer one character at a time
 * (a branch plus a byte store per char, verified in generated code). Fill via
 * @ref writable / @ref commit / @ref allocate instead.
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

    [[nodiscard]] auto data() const noexcept -> const char*
    {
        return mHeapStorage.empty() ? mInlineStorage.data()
                                    : mHeapStorage.data();
    }
    [[nodiscard]] auto size() const noexcept -> std::size_t { return mLength; }
    [[nodiscard]] auto view() const noexcept -> std::string_view
    {
        return {data(), size()};
    }

private:
    std::array<char, InlineCapacity> mInlineStorage{};
    std::vector<char> mHeapStorage;
    std::size_t mLength = 0;
};

/**
 * @brief Two-pass fill: produce into the inline region, retry on the heap only
 * if it does not fit.
 *
 * @p produce takes the destination span and returns the needed length; it may
 * write a truncated prefix on the first pass.
 */
template <std::size_t InlineCapacity, class Producer>
auto format_two_pass(LoggerBuffer<InlineCapacity>& buffer, Producer&& produce)
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
