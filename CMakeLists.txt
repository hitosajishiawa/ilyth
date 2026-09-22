cmake_minimum_required(VERSION 3.25...3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    ilyth
    VERSION 0.1.0
    LANGUAGES CXX C
)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(ILYTH_BUILD_EXAMPLES "build examples" ON)
option(ILYTH_BUILD_TESTS    "build tests"    ON)
option(ILYTH_BUILD_TOOLS    "build tools"    ON)

if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build Type: (multi-config) ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
endif()

# Compilation jobs
cmake_host_system_information(RESULT _cpu_num QUERY NUMBER_OF_LOGICAL_CORES)
message(STATUS "Number of CPU logical cores on current server: ${_cpu_num}")
math(EXPR _jobs "${_cpu_num} * 1 / 4")  # Depends on your machine. Use a larger ratio if you have enough memory.
set(CMAKE_BUILD_PARALLEL_LEVEL ${_jobs} CACHE STRING "Number of threads for parallel compilation")
message(STATUS "Compiler default work jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")

# Include compilation options
include(cmake/IlythOptions.cmake)

# Subdirectories
add_subdirectory(ilyth)
if(ILYTH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(ILYTH_BUILD_TOOLS)
    add_subdirectory(tools)
endif()
