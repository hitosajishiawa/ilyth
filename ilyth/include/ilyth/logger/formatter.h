#ifndef ILYTH_LOGGER_FORMATTER_H
#define ILYTH_LOGGER_FORMATTER_H

#include <ilyth/logger/entry.h>
#include <ilyth/logger/timestamp.h>

#include <cstddef>
#include <format>
#include <span>

namespace ilyth
{

/**
 * @brief Signature of a formatter: render @p entry into @p out .
 *
 * Returns how many characters the entry needs. When that exceeds `out.size()`
 * the caller re-invokes it with an exactly sized buffer (two-pass), so a
 * formatter must tolerate writing a truncated prefix on the first pass.
 *
 * Formatters are stateless, so any function with this signature works; group
 * several in a namespace and pass their address:
 * ```
 * namespace fmts
 * {
 * auto compact(const LogEntry&, std::span<char>) -> std::size_t;
 * auto verbose(const LogEntry&, std::span<char>) -> std::size_t;
 * }
 * logger.setFormatter(&fmts::compact);
 * ```
 * A struct works too, but its members must be `static`; non-static members are
 * member pointers and can't be converted to `EntryFormatter`.
 */
using EntryFormatter = std::size_t (*)(const LogEntry& entry,
                                       std::span<char> out);

/**
 * @brief Default formatter:
 * `[<timestamp> ; <elapsed>] [<level>] "<file>:<line>"
 * [thread:<threadName>(id=<>threadID), fiber_id=<fiberID>] <message>\n`.
 *
 * `<time>` precision is @ref detail::DefaultTimestampDigits .
 */
inline auto defaultFormatter(const LogEntry& e, std::span<char> out)
    -> std::size_t
{
    const auto result = std::format_to_n(
        out.data(), static_cast<std::ptrdiff_t>(out.size()),
        "[{} ; {}] [{}] \"{}:{}\" [thread:{}(id={}), fiber_id={}] {}\n",
        Timestamp{e.timestamp}, e.elapsed, e.level, e.location.file_name(),
        e.location.line(), e.threadName, e.threadID, e.fiberID, e.message);
    return static_cast<std::size_t>(result.size);
}

}  // namespace ilyth

#endif
