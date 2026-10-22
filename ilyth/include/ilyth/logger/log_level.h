#ifndef ILYTH_LOGGER_LOGLEVEL_H
#define ILYTH_LOGGER_LOGLEVEL_H

#include <ilyth/export.h>
#include <ilyth/types.h>

#include <format>
#include <string_view>

namespace ilyth
{

class ILYTH_API LogLevel final
{
public:
    /**
     * @brief Log verbosity, ordered from least to most severe.
     *
     * @internal
     * Do not explicitly assign values to the enumerators: the metaprogramming
     * tricks (to be deprecated in C++26) and the ordering rule below both break
     * if you do. Declarations are ordered from least to most important, with
     * @ref OFF last; `Logger::shouldLog` and the level-name table rely on this.
     */
    enum class Level : types::u8
    {
        /**
         * @brief Debug information, such as variable values.
         */
        DEBUG,
        /**
         * @brief Normal progress, such as "server launching..." or "database
         * linked successfully".
         */
        INFO,
        /**
         * @brief Exceptions or potential bugs that neither break subsequent
         * operations nor disturb other components.
         */
        WARN,
        /**
         * @brief Operation failures or exceptions that need investigation.
         */
        ERROR,
        /**
         * @brief Fatal errors.
         *
         * It is not recommended to continue after one; execution is usually
         * followed by termination.
         */
        FATAL,
        /**
         * @brief Disable logging, e.g. in benchmarks.
         */
        OFF,

        /**
         * @brief Sentinel equal to the number of real levels.
         *
         * Only for internal implementations: it exists so that they can
         * `static_assert` that their level-name table stays in sync with this
         * enum.
         *
         * @internal Keep it last and implicit so that its value is the count.
         *
         * @warning Not a valid level; never use it in a log call.
         *
         * @todo C++26: deprecate it.
         */
        _COUNT,
    };

    /**
     * @brief Get the name of @p lv .
     *
     * @return The enumerator name, or `"UNKNOWN"` if @p lv is not valid.
     */
    [[nodiscard]] static auto ToString(Level lv) noexcept -> std::string_view;
    /**
     * @brief Parse a level name produced by @ref ToString .
     *
     * @return The matching level, or @ref Level::OFF as the fallback if
     * @p value is unknown.
     */
    [[nodiscard]] static auto FromString(std::string_view value) noexcept
        -> Level;
};

}  // namespace ilyth

/**
 * Lets `std::format` render a level by its name, e.g. `"{}"` -> `"INFO"`.
 * Declared before any use so `std::format_to_n` below picks it up.
 */
template <>
struct std::formatter<ilyth::LogLevel::Level, char>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();  // No spec supported; must sit right before '}'.
    }

    auto format(ilyth::LogLevel::Level level, std::format_context& ctx) const
    {
        const auto name = ilyth::LogLevel::ToString(level);
        return std::copy(name.begin(), name.end(), ctx.out());
    }
};

#endif
