#ifndef ILYTH_PLATFORM_H
#define ILYTH_PLATFORM_H

// Introduce `__GLIBC__` if exists
#if defined(__linux__) && __has_include(<features.h>)
    #include <features.h>
#endif

#if defined(__linux__) && defined(__GLIBC__)
    #define ILYTH_PLATFORM_LINUX_GLIBC 1
#else
    #define ILYTH_PLATFORM_LINUX_GLIBC 0
#endif

// Linux regardless of the C library (glibc, musl, ...).
#if defined(__linux__)
    #define ILYTH_PLATFORM_LINUX 1
#else
    #define ILYTH_PLATFORM_LINUX 0
#endif

#if defined(_WIN32)
    #define ILYTH_PLATFORM_WIN32 1
#else
    #define ILYTH_PLATFORM_WIN32 0
#endif

// Architecture, predefined by compiler.
// macros, so the checks do not depend on include order.
#if defined(__x86_64__) || defined(__i386__)
    #define ILYTH_PLATFORM_ARCH_X86 1
#else
    #define ILYTH_PLATFORM_ARCH_X86 0
#endif

// Covers both AArch64 and 32-bit ARM.
#if defined(__aarch64__) || defined(__arm__)
    #define ILYTH_PLATFORM_ARCH_ARM 1
#else
    #define ILYTH_PLATFORM_ARCH_ARM 0
#endif

// Nothing in the library knows how to hint the CPU on this architecture yet.
#if !ILYTH_PLATFORM_ARCH_X86 && !ILYTH_PLATFORM_ARCH_ARM
    #define ILYTH_PLATFORM_ARCH_UNKNOWN 1
#else
    #define ILYTH_PLATFORM_ARCH_UNKNOWN 0
#endif

#endif
