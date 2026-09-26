#include <ilyth/assert.h>
#include <ilyth/types.h>

#include <array>
#include <cstdlib>
#include <print>
#include <span>

using namespace ilyth::types;

// Every function below runs the *same* check over the *same* data. The only
// difference is the statement written after a failed check, which is the whole
// point: the macro expands to a plain call, so `break`, `continue` and
// `return` keep their usual meaning instead of hitting a wrapper loop.
namespace
{

constexpr auto Records = std::array<s64, 6>{1, 2, -3, 4, -5, 6};

// (1) example for *assert_break*
auto sumUntilBad(std::span<const s64> values)
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
auto sumSkippingBad(std::span<const s64> values)
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
auto sumOrZero(std::span<const s64> values)
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
auto sumIgnoringBad(std::span<const s64> values)
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
        "Compiled in release mode. Assert failures will only bring logs.");
#endif

    std::println("records = {{1, 2, -3, 4, -5, 6}}");
    std::println("[1] break    -> sum until the first bad record = {}",
                 sumUntilBad(Records));
    std::println("[2] continue -> sum skipping bad records       = {}",
                 sumSkippingBad(Records));
    std::println("[3] return   -> all-or-nothing sum             = {}",
                 sumOrZero(Records));
    std::println("[4] assert   -> sum ignoring bad records       = {}",
                 sumIgnoringBad(Records));

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
