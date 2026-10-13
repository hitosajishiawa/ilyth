#include <ilyth/logger/log_level.h>

#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

/// X-macro: the only place in the *implementation* that knows the ordered
/// list of level names. It will be unregistered at the end of this file.
///
/// @note Manually keep it sync with definitions in `LogLevel::Level`.
///
/// @todo C++26: replace this X-macro (and the two `static_assert`s plus the
/// `_COUNT` sentinel that exist only to keep it honest) with static reflection:
/// `std::meta::enumerators_of(^Level)` + `std::define_static_array`. The public
/// `ToString`/`FromString` signatures stay unchanged, so the swap is local.
#define ILYTH_LOG_LEVEL_LIST(f) \
    f(DEBUG) f(INFO) f(WARN) f(ERROR) f(FATAL) f(OFF)

namespace
{

// Length of the list: expands to `0 +1 +1 ...`.
#define ILYTH_LEVEL_COUNT_ONE(name) +1
constexpr auto LevelCount =
    std::size_t{0 ILYTH_LOG_LEVEL_LIST(ILYTH_LEVEL_COUNT_ONE)};
#undef ILYTH_LEVEL_COUNT_ONE

// Underlying value of every level, in list order: `{ 0, 1, 2, ... }`.
constexpr auto LevelValues = std::array<std::size_t, LevelCount>{
#define ILYTH_LEVEL_VALUE(name) \
    std::to_underlying(ilyth::LogLevel::Level::name),
    ILYTH_LOG_LEVEL_LIST(ILYTH_LEVEL_VALUE)
#undef ILYTH_LEVEL_VALUE
};

// `LevelValues` is exactly `0, 1, ..., LevelCount - 1`.
consteval auto IsIdentitySequence() noexcept -> bool
{
    for (std::size_t i = 0; i < LevelValues.size(); ++i)
    {
        if (LevelValues[i] != i)
        {
            return false;
        }
    }
    return true;
}

}  // namespace

/// The two `static_assert`s below are what keep the hand-written enum and this
/// list in sync: no missing name, no extra name, and the same order.

// (1) Same number of levels: list length == number of real enumerators.
static_assert(LevelCount == std::to_underlying(ilyth::LogLevel::Level::_COUNT),
              "ILYTH_LOG_LEVEL_LIST is out of sync with LogLevel::Level: the "
              "enum has a different number of levels than the list has names.");

// (2) Same order and values: name #i maps to underlying value i. Together
// with (1) this proves the list is a bijection onto the enum: every name is a
// real enumerator (otherwise the `case` below fails to compile), every name is
// distinct (duplicate `case` labels are a hard error), and the values they
// cover are exactly [0, LevelCount).
static_assert(IsIdentitySequence(),
              "ILYTH_LOG_LEVEL_LIST is out of sync with LogLevel::Level: the "
              "order or the underlying values differ.");

namespace ilyth
{

auto LogLevel::ToString(Level lv) noexcept -> std::string_view
{
    switch (lv)
    {
#define XX(name) \
    case Level::name: return #name;
        ILYTH_LOG_LEVEL_LIST(XX)
#undef XX
    case Level::_COUNT: break;  // Sentinel, not a real level.
    }
    return "UNKNOWN";
}

auto LogLevel::FromString(std::string_view value) noexcept -> LogLevel::Level
{
#define XX(name)            \
    if (value == #name)     \
    {                       \
        return Level::name; \
    }
    ILYTH_LOG_LEVEL_LIST(XX)
#undef XX
    return Level::OFF;  // fallback
}

}  // namespace ilyth

#undef ILYTH_LOG_LEVEL_LIST
