file(GLOB ILYTH_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

list(SORT ILYTH_TEST_SOURCES)

set(ILYTH_TEST_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin/tests")

foreach(test_source IN LISTS ILYTH_TEST_SOURCES)
    get_filename_component(test_name "${test_source}" NAME_WE)

    if(TARGET "${test_name}")
        message(FATAL_ERROR "Cannot create test target '${test_name}': a target with that name already exists")
    endif()

    add_executable("${test_name}" "${test_source}")
    target_link_libraries("${test_name}" PRIVATE ilyth::ilyth ilyth_executable_options)
    set_target_properties("${test_name}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ILYTH_TEST_OUTPUT_DIR}")

    foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${config}" config_upper)
        set_target_properties("${test_name}" PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${ILYTH_TEST_OUTPUT_DIR}"
        )
    endforeach()

    add_test(NAME "${test_name}" COMMAND "${test_name}")
endforeach()
