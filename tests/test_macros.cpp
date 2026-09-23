#include <ilyth/macros.h>
#include <cstdlib>
#include <print>

using namespace ilyth::types;

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

    auto pow = [](s64 x, u8 p) {
        auto sign = (x < 0) && (p % 2 == 1);
        auto re   = std::abs(x);
        while (p != 0)
        {
            re *= x;
            --p;
            ilyth_assert2(re < 0, "Pow overflow");
        }
        return re * (sign ? -1 : 1);
    };
    pow(-2, 9);
    pow(42, 9);
}
