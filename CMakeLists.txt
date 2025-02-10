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
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)

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

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

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
# Set the library to use c++-11
target_compile_features(json PUBLIC cxx_std_11)

# =====================================
# COMPILATION FLAGS
# =====================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(json PUBLIC _CRT_SECURE_NO_WARNINGS)

    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)

    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(json PUBLIC /WX)
    endif()

    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(json PUBLIC /experimental:external)
        target_compile_options(json PUBLIC /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(json PUBLIC /external:anglebrackets)
        target_compile_options(json PUBLIC /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(json PUBLIC /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(json PUBLIC /permissive-)
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(json PUBLIC -Werror)
    endif()

    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(json PUBLIC
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )

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
# CODE ANALYSIS
# -----------------------------------------------------------------------------

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE ALL_PROJECT_FILES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )
    add_custom_target(
        json_clang_tidy
        COMMAND ${CLANG_TIDY_EXE}
        --p=${CMAKE_BINARY_DIR}
        ${ALL_PROJECT_FILES}
        COMMENT "Running clang-tidy"
        VERBATIM
    )
    add_custom_target(
        json_clang_tidy_fix
        COMMAND ${CLANG_TIDY_EXE}
        --fix
        --p=${CMAKE_BINARY_DIR}
        ${ALL_PROJECT_FILES}
        COMMENT "Running clang-tidy-fix"
        VERBATIM
    )
endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)

    # FetchContent: Doxygen Awesome CSS
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    FetchContent_MakeAvailable(doxygenawesome)

    # Hide FetchContent variables to avoid clutter in ccmake.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # Read the file with the version.
    file(READ ${PROJECT_SOURCE_DIR}/include/json/json.hpp version_file)
    # Extract the version.
    string(REGEX MATCH "JSON_MAJOR_VERSION ([0-9]*)" _ ${version_file})
    set(JSON_MAJOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "JSON_MINOR_VERSION ([0-9]*)" _ ${version_file})
    set(JSON_MINOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "JSON_MICRO_VERSION ([0-9]*)" _ ${version_file})
    set(JSON_MICRO_VERSION ${CMAKE_MATCH_1})

    # Customization: Doxygen Configuration
    set(DOXYGEN_WARN_FORMAT "$file:$line:1: $text")
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_BRIEF "A modern JSON library for C++")
    set(DOXYGEN_PROJECT_NUMBER "${JSON_MAJOR_VERSION}.${JSON_MINOR_VERSION}.${JSON_MICRO_VERSION}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_LATEX NO) # Disable LaTeX if not needed
    set(DOXYGEN_GENERATE_MAN NO)   # Disable man pages if not needed

    # Styling and UX enhancements using Doxygen Awesome
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )

    # Set stricter warnings for better documentation quality
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR YES)
    set(DOXYGEN_WARN_NO_PARAMDOC YES)
    set(DOXYGEN_WARN_AS_ERROR YES) # Treat warnings as errors for CI

    # Exclude certain files or directories from documentation (if needed)
    set(DOXYGEN_EXCLUDE_PATTERNS "${PROJECT_SOURCE_DIR}/tests/*" "${PROJECT_SOURCE_DIR}/examples/*")

    file(GLOB_RECURSE ALL_PROJECT_FILES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )

    # Add Doxygen documentation target.
    doxygen_add_docs(
        ${PROJECT_NAME}_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_HEADERS}
        ${PROJECT_SOURCES}
        COMMENT "Generating Doxygen documentation for the library"
    )
endif()
