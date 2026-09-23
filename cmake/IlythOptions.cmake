add_library(ilyth_options INTERFACE)

target_compile_options(ilyth_options INTERFACE
    # gcc/clang
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall;-Wextra;-Wpedantic;-fno-strict-aliasing>
)
target_compile_options(ilyth_options INTERFACE
    # for linkage
    $<$<AND:$<PLATFORM_ID:Linux>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-fPIC>
)

add_library(ilyth_executable_options INTERFACE)
target_link_options(ilyth_executable_options INTERFACE
    # for stacktrace
    $<$<AND:$<PLATFORM_ID:Linux>,$<LINK_LANG_AND_ID:CXX,GNU,Clang>>:-rdynamic>
)

# Debug
target_compile_definitions(ilyth_options INTERFACE
    $<$<CONFIG:Debug>:ILYTH_DEBUG>
)
target_compile_options(ilyth_options INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-O0;-g3;-ggdb>
)

# Release & RelWithDebInfo
# NOTE: CMake's default CMAKE_CXX_FLAGS_<CONFIG> already defines NDEBUG here;
# kept explicit on purpose.
target_compile_definitions(ilyth_options INTERFACE
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG>
)
target_compile_options(ilyth_options INTERFACE
    $<$<AND:$<CONFIG:Release>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-O2;-fno-omit-frame-pointer>
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-O2;-g;-fno-omit-frame-pointer>
)

# Coverage: default OFF due to significant performance overhead
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(ilyth_options INTERFACE
        $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:--coverage>
    )
    target_link_options(ilyth_options INTERFACE
        $<$<AND:$<CONFIG:Debug>,$<LINK_LANG_AND_ID:CXX,GNU,Clang>>:--coverage>
    )
endif()
