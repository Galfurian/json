# -----------------------------------------------------------------------------
# @brief  : Root cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# Set the minimum CMake version, the project name and default build type.
cmake_minimum_required(VERSION 3.1...3.18)

# Set the project name.
project(json CXX)

# Set the default build type to Debug.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# -----------------------------------------------------------------------------
# OPTIONS
# -----------------------------------------------------------------------------

option(STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_TESTS "Build tests" OFF)

# -----------------------------------------------------------------------------
# ENABLE FETCH CONTENT
# -----------------------------------------------------------------------------

# We need this in order to import external projects.
include(FetchContent)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_QUIET
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_UPDATES_DISCONNECTED
)

# -----------------------------------------------------------------------------
# DEPENDENCY (SYSTEM LIBRARIES)
# -----------------------------------------------------------------------------

find_package(Doxygen)

# -----------------------------------------------------------------------------
# DIPENDENCIES (REPOSITORIES)
# -----------------------------------------------------------------------------

# Retrieve the source.
FetchContent_Declare(ordered_map
    GIT_REPOSITORY "https://github.com/Galfurian/ordered_map.git"
    GIT_TAG main
)
# Retrieve the properties related to the content.
FetchContent_MakeAvailable(ordered_map)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_UPDATES_DISCONNECTED_ORDERED_MAP
    FETCHCONTENT_SOURCE_DIR_ORDERED_MAP
)

# Retrieve the source.
FetchContent_Declare(timelib
    GIT_REPOSITORY "https://github.com/Galfurian/timelib.git"
    GIT_TAG main
)
# Retrieve the properties related to the content.
FetchContent_MakeAvailable(timelib)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_UPDATES_DISCONNECTED_TIMELIB
    FETCHCONTENT_SOURCE_DIR_TIMELIB
)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(json ${PROJECT_SOURCE_DIR}/src/json.cpp)
add_library(json::json ALIAS json)

# Include header directories, and link libraries.
target_include_directories(json PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(json PUBLIC ${ordered_map_SOURCE_DIR}/include)

target_compile_options(json PRIVATE -fPIC)

# =====================================
# COMPILATION FLAGS
# =====================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(json PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(json PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(json PRIVATE /experimental:external)
        target_compile_options(json PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(json PRIVATE /external:anglebrackets)
        target_compile_options(json PRIVATE /external:W0)
        target_compile_options(json PRIVATE /W4)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        target_compile_options(json PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        target_compile_options(json PRIVATE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(BUILD_EXAMPLES)

    # Add the example.
    add_executable(json_example examples/example.cpp)
    # Set C++11 standard for this target
    set_target_properties(json_example PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Include header directories.
    target_include_directories(json_example PUBLIC ${timelib_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(json_example PUBLIC json timelib)

    # Add the example.
    add_executable(json_run_on_file examples/run_on_file.cpp)
    # Set C++11 standard for this target
    set_target_properties(json_run_on_file PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Include header directories.
    target_include_directories(json_run_on_file PUBLIC ${timelib_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(json_run_on_file PUBLIC json timelib)

endif(BUILD_EXAMPLES)

# -----------------------------------------------------------------------------
# TESTS
# -----------------------------------------------------------------------------

if(BUILD_TESTS)

    # CMake has support for adding tests to a project.
    enable_testing()

    # Add the test.
    add_executable(json_test_data_types ${PROJECT_SOURCE_DIR}/tests/test_data_types.cpp)
    add_test(json_test_data_types_run json_test_data_types)
    # Set C++11 standard for this target
    set_target_properties(json_test_data_types PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_data_types json)

    # Add the test.
    add_executable(json_test_struct ${PROJECT_SOURCE_DIR}/tests/test_struct.cpp)
    add_test(json_test_struct_run json_test_struct)
    # Set C++11 standard for this target
    set_target_properties(json_test_struct PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_struct json)

    # Add the test.
    add_executable(json_test_container ${PROJECT_SOURCE_DIR}/tests/test_container.cpp)
    add_test(json_test_container_run json_test_container)
    # Set C++11 standard for this target
    set_target_properties(json_test_container PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_container json)

    # Add the test.
    add_executable(json_test_friend ${PROJECT_SOURCE_DIR}/tests/test_friend.cpp)
    add_test(json_test_friend_run json_test_friend)
    # Set C++11 standard for this target
    set_target_properties(json_test_friend PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_friend json)

    # Add the test.
    add_executable(json_test_order ${PROJECT_SOURCE_DIR}/tests/test_order.cpp)
    add_test(json_test_order_run json_test_order)
    # Set C++11 standard for this target
    set_target_properties(json_test_order PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_order json)

    # Add the test.
    add_executable(json_test_error ${PROJECT_SOURCE_DIR}/tests/test_error.cpp)
    add_test(json_test_error_run json_test_error)
    # Set C++11 standard for this target
    set_target_properties(json_test_error PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_error json)

    # Add the test.
    add_executable(json_test_json5 ${PROJECT_SOURCE_DIR}/tests/test_json5.cpp)
    add_test(json_test_json5_run json_test_json5)
    # Set C++11 standard for this target
    set_target_properties(json_test_json5 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_json5 json)

    # Add the test.
    add_executable(json_test_pointer ${PROJECT_SOURCE_DIR}/tests/test_pointer.cpp)
    add_test(json_test_pointer_run json_test_pointer)
    # Set C++11 standard for this target
    set_target_properties(json_test_pointer PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    # Linking for the test.
    target_link_libraries(json_test_pointer json)

endif(BUILD_TESTS)

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # Record the options that describe how to populate the specified content.
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    # Retrieve the properties related to the content.
    FetchContent_MakeAvailable(doxygenawesome)
    # Hide fetchcontent variables, otherwise with ccmake it's a mess.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # = CUSTOMIZATION =========================================================
    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    set(DOXYGEN_PROJECT_NAME json)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )
    doxygen_add_docs(
        json_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_SOURCE_DIR}/include/json/json.hpp
        ${PROJECT_SOURCE_DIR}/src/json.cpp
    )
endif()
