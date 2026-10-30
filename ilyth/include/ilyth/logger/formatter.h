#ifndef ILYTH_LOGGER_FORMATTER_H
#define ILYTH_LOGGER_FORMATTER_H

#include <ilyth/logger/entry.h>
#include <ilyth/logger/time.h>

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
 * Formatters are stateless, so any function with this signature works. A good
 * practice is to group several in a namespace and to pass their address:
 * ```
 * namespace fmts
 * {
 * auto compact(const LogEntry&, std::span<char>) -> std::size_t;
 * auto verbose(const LogEntry&, std::span<char>) -> std::size_t;
 * }
 * logger.setFormatter(&fmts::compact);
 * ```
 * A struct with static members works too. Choose the one you prefer.
 */
using EntryFormatter = std::size_t (*)(const LogEntry& entry,
                                       std::span<char> out);

/**
 * @brief Default verbose formatter, showcasing all data members of @p e :
 * `[<timestamp> ; <elapsed>] [<level>] "<file>:<line>" [thread:<threadName>
 * (id=<threadID>), fiber_id=<fiberID>]\n  <message>\n`.
 *
 * `<timestamp>` renders as `YYYY-MM-DD HH:MM:SS.fff` ( @ref Timestamp ) and
 * `<elapsed>` as `[<days>d ]HH:MM:SS.fff` since process start ( @ref Elapsed ).
 */
inline auto defaultVerboseFormatter(const LogEntry& e, std::span<char> out)
    -> std::size_t
{
    const auto result = std::format_to_n(
        out.data(), static_cast<std::ptrdiff_t>(out.size()),
        "[{} ; {}] [{}] \"{}:{}\" [thread:{} (id={}), fiber_id={}]\n  {}\n",
        e.timestamp, e.elapsed, e.level, e.location.file_name(),
        e.location.line(), e.threadName, e.threadID, e.fiberID, e.message);
    return static_cast<std::size_t>(result.size);
}

/**
 * @brief Default compact formatter:
 * `[<timestamp>] [<level>] "<file>:<line>" <message>\n`
 * where `<timestamp>` renders as `YYYY-MM-DD HH:MM:SS.f`.
 */
inline auto defaultCompactFormatter(const LogEntry& e, std::span<char> out)
    -> std::size_t
{
    const auto result =
        std::format_to_n(out.data(), static_cast<std::ptrdiff_t>(out.size()),
                         "[{:.1}] [{}] \"{}:{}\" {}\n", e.timestamp, e.level,
                         e.location.file_name(), e.location.line(), e.message);
    return static_cast<std::size_t>(result.size);
}

}  // namespace ilyth

#endif
