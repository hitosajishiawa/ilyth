/**
 * Private wrapper around <windows.h>: keeps the two opt-outs in one place so
 * every translation unit that pulls in Windows headers does so consistently.
 *
 * - WIN32_LEAN_AND_MEAN: skip rarely used subsystems (winsock 1.1, RPC, DDE,
 *   shell, ...) to cut compile time and namespace pollution.
 * - NOMINMAX: keep <windows.h> from defining min/max macros that break
 *   std::min/std::max and friends.
 *
 * Deliberately private: defining these in a public header would impose them on
 * downstream translation units and would only take effect if they happen to be
 * included before <windows.h>.
 */

#ifndef ILYTH_SRC_PLATFORM_WIN32_H
#define ILYTH_SRC_PLATFORM_WIN32_H

#include <ilyth/platform.h>

#ifndef ILYTH_PLATFORM_WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

#endif
