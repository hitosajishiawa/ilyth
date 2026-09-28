#ifndef ILYTH_LOGGER_LOGLEVEL_H
#define ILYTH_LOGGER_LOGLEVEL_H

#include <ilyth/export.h>
#include <ilyth/types.h>

#include <string_view>

namespace ilyth
{

class ILYTH_API LogLevel final
{
public:
    /**
     * @note DO NOT explicit assign values to enumerators, or the
     * metaprogramming tricks will broken.
     */
    enum class Level : types::u8
    {
        /**
         * Mark debug related informations. For example, the value of vars.
         */
        DEBUG,
        /**
         * Mark running smoothly. For example, database link successfully.
         */
        INFO,
        /**
         * Mark exceptions or potential bugs that won't break following
         * operations or disturb other functions.
         */
        WARN,
        /**
         * Mark operation failures or exceptions that need examinations.
         */
        ERROR,
        /**
         * Mark fatal errors. If a fatal error occurs, it is not suggested to
         * go on.
         */
        FATAL,
        /**
         * Off the log in some situations, such as benchmark.
         */
        OFF,

        /**
         * Sentinel, equals the number of real log levels for metaprogramming
         * tricks. Only for internal implementations.
         *
         * It exists only so that the implementation can `static_assert` that
         * its own level name list stays in sync with this enum.
         *
         * @note Keep it last and implicit so its value is the count.
         *
         * @warning NOT a valid level. You should never use it in log.
         */
        _COUNT,
    };

    [[nodiscard]] static auto ToString(Level lv) noexcept -> std::string_view;
    [[nodiscard]] static auto FromString(std::string_view value) noexcept
        -> Level;
};

}  // namespace ilyth

#endif
