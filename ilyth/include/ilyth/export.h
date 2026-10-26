/**
 * @file Utils for controlling symbols to be export and import.
 */

#ifndef ILYTH_EXPORT_H
#define ILYTH_EXPORT_H

#if defined(ILYTH_SHARED_LIB)  // Shared library.
                               // CURRENTLY UNDEFINED, TODO

    #if ILYTH_PLATFORM_WIN32  // Windows
        #if defined(ILYTH_BUILDING_LIB)
            #define ILYTH_API __declspec(dllexport)
        #else
            #define ILYTH_API __declspec(dllimport)
        #endif
    #elif defined(__GNUC__)  // Linux
        #define ILYTH_API __attribute__((visibility("default")))
    #endif

#else  // Static library

    #define ILYTH_API  // do nothing

#endif

#endif
