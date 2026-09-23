#include <ilyth/macros.h>

#include <array>
#include <cstdlib>
#include <limits>
#include <print>
#include <span>

using namespace ilyth::types;

namespace
{

// Every function below runs the *same* check over the *same* data. The only
// difference is the statement written after a failed check, which is the whole
// point: the macro expands to a plain call, so `break`, `continue` and
// `return` keep their usual meaning instead of hitting a wrapper loop.

constexpr auto Records = std::array<s64, 6>{1, 2, -3, 4, -5, 6};

// (1) example for *assert_break*
auto sum_until_bad(std::span<const s64> values)
{
    auto sum = s64{};
    for (auto v : values)
    {
        if (!ilyth_check2(v >= 0, "record must be non-negative"))
        {
            break;
        }
        sum += v;
    }
    return sum;
}

// (2) example for *assert_continue*
auto sum_skipping_bad(std::span<const s64> values)
{
    auto sum = s64{};
    for (auto v : values)
    {
        if (!ilyth_check2(v >= 0, "record must be non-negative"))
        {
            continue;
        }
        sum += v;
    }
    return sum;
}

// (3) example for *assert_return*
auto sum_or_zero(std::span<const s64> values)
{
    auto sum = s64{};
    for (auto v : values)
    {
        if (!ilyth_check2(v >= 0, "record must be non-negative"))
        {
            return s64{};  // leaves the function, not just the loop
        }
        sum += v;
    }
    return sum;
}

// (4) example for *assert_noeffect*
auto sum_ignoring_bad(std::span<const s64> values)
{
    auto sum = s64{};
    for (auto v : values)
    {
        ilyth_assert2(v >= 0, "record must be non-negative");
        sum += v;
    }
    return sum;
}

}  // namespace

/* Test assert in various contexts */
int main()
{
#ifdef ILYTH_DEBUG
    std::println(
        "Compiled in debug mode. Assert failure will lead to termination.");
#endif

#ifdef NDEBUG
    std::println(
        "Compiled in release mode. Assert failure will only bring logs.");
#endif

    std::println("records = {{1, 2, -3, 4, -5, 6}}");
    std::println("[1] break    -> sum until the first bad record = {}",
                 sum_until_bad(Records));
    std::println("[2] continue -> sum skipping bad records       = {}",
                 sum_skipping_bad(Records));
    std::println("[3] return   -> all-or-nothing sum             = {}",
                 sum_or_zero(Records));
    std::println("[4] assert   -> sum ignoring bad records       = {}",
                 sum_ignoring_bad(Records));

    // `ilyth_check` (no extra info) used as a plain guard clause.
    auto checked_abs = [](s64 x) -> s64 {
        if (!ilyth_check(x != min_s64))
        {
            return 0;
        }
        return std::abs(x);
    };
    std::println("[5] guard    -> checked_abs(INT64_MIN)         = {}",
                 checked_abs(min_s64));
}
