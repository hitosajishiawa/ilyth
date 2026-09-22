add_library(ilyth_options INTERFACE)

target_compile_options(ilyth_options INTERFACE
    # gcc/clang
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall;-Wextra;-Wpedantic;-fno-strict-aliasing>
)
target_compile_options(ilyth_options INTERFACE
    # for linkage
    $<$<AND:$<PLATFORM_ID:Linux>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-fPIC>
)

target_link_options(ilyth_options INTERFACE
    # for stacktrace
    $<$<AND:$<PLATFORM_ID:Linux>,$<LINK_LANG_AND_ID:CXX,GNU,Clang>>:-rdynamic>
)

# Debug
target_compile_options(ilyth_options INTERFACE
    $<$<CONFIG:Debug>:ILYTH_DEBUG>
)
target_compile_options(ilyth_options INTERFACE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-ggdb>
)

# Release & RelWithDebInfo
target_compile_options(ilyth_options INTERFACE
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-fno-omit-frame-pointer>

    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g>
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
)

# Coverage: default OFF due to significant performance overhead
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(ilyth_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
    target_link_options(ilyth_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
endif()
