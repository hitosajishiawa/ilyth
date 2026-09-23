file(GLOB_RECURSE ILYTH_SOURCES CONFIGURE_DEPENDS *.cpp *.c *.cc)
file(GLOB_RECURSE ILYTH_HEADERS CONFIGURE_DEPENDS *.h *.hpp)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library(ilyth STATIC ${ILYTH_SOURCES} ${ILYTH_HEADERS})
add_library(ilyth::ilyth ALIAS ilyth)

target_include_directories(ilyth
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(ilyth PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Werror>
)

target_link_libraries(ilyth
    PUBLIC
        ilyth_options
        Threads::Threads
)

install(TARGETS ilyth
    EXPORT ilythTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ilyth
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
