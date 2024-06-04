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
    GIT_TAG 8a520162623b1e2bd03f9398c42a6e380064e4a8
)
# Retrieve the properties related to the content.
FetchContent_MakeAvailable(ordered_map)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_UPDATES_DISCONNECTED_ORDERED_MAP
    FETCHCONTENT_SOURCE_DIR_ORDERED_MAP
)

# Retrieve the source.
FetchContent_Declare(stopwatch
    GIT_REPOSITORY "https://github.com/Galfurian/stopwatch.git"
    GIT_TAG a0e98edc32a60e7210a25fccc0e1e9222f16bdac
)
# Retrieve the properties related to the content.
FetchContent_MakeAvailable(stopwatch)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_UPDATES_DISCONNECTED_STOPWATCH
    FETCHCONTENT_SOURCE_DIR_STOPWATCH
)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/json.cpp)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Include header directories, and link libraries.
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${ordered_map_SOURCE_DIR}/include)

target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

# =====================================
# COMPILATION FLAGS
# =====================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} PRIVATE /WX)
    endif()
    if(STRICT_WARNINGS)
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(${PROJECT_NAME} PRIVATE /experimental:external)
        target_compile_options(${PROJECT_NAME} PRIVATE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(${PROJECT_NAME} PRIVATE /external:anglebrackets)
        target_compile_options(${PROJECT_NAME} PRIVATE /external:W0)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
    endif()
    if(STRICT_WARNINGS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(BUILD_EXAMPLES)

    # Add the example.
    add_executable(${PROJECT_NAME}_example examples/example.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_example PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_example PUBLIC ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME}_example PUBLIC stopwatch)
    
    # Add the example.
    add_executable(${PROJECT_NAME}_run_on_file examples/run_on_file.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_run_on_file PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_run_on_file PUBLIC ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME}_run_on_file PUBLIC stopwatch)
    
endif(BUILD_EXAMPLES)

# -----------------------------------------------------------------------------
# TESTS
# -----------------------------------------------------------------------------

if(BUILD_TESTS)

    # CMake has support for adding tests to a project.
    enable_testing()

    # Add the test.
    add_executable(${PROJECT_NAME}_test_data_types ${PROJECT_SOURCE_DIR}/tests/test_data_types.cpp)
    add_test(${PROJECT_NAME}_test_data_types_run ${PROJECT_NAME}_test_data_types)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_data_types ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_struct ${PROJECT_SOURCE_DIR}/tests/test_struct.cpp)
    add_test(${PROJECT_NAME}_test_struct_run ${PROJECT_NAME}_test_struct)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_struct ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_container ${PROJECT_SOURCE_DIR}/tests/test_container.cpp)
    add_test(${PROJECT_NAME}_test_container_run ${PROJECT_NAME}_test_container)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_container ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_friend ${PROJECT_SOURCE_DIR}/tests/test_friend.cpp)
    add_test(${PROJECT_NAME}_test_friend_run ${PROJECT_NAME}_test_friend)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_friend ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_order ${PROJECT_SOURCE_DIR}/tests/test_order.cpp)
    add_test(${PROJECT_NAME}_test_order_run ${PROJECT_NAME}_test_order)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_order ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_error ${PROJECT_SOURCE_DIR}/tests/test_error.cpp)
    add_test(${PROJECT_NAME}_test_error_run ${PROJECT_NAME}_test_error)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_error ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_json5 ${PROJECT_SOURCE_DIR}/tests/test_json5.cpp)
    add_test(${PROJECT_NAME}_test_json5_run ${PROJECT_NAME}_test_json5)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_json5 ${PROJECT_NAME})

    # Add the test.
    add_executable(${PROJECT_NAME}_test_pointer ${PROJECT_SOURCE_DIR}/tests/test_pointer.cpp)
    add_test(${PROJECT_NAME}_test_pointer_run ${PROJECT_NAME}_test_pointer)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_pointer ${PROJECT_NAME})

endif(BUILD_TESTS)

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # Record the options that describe how to populate the specified content.
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG df88fe4fdd97714fadfd3ef17de0b4401f804052
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
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
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
        ${PROJECT_NAME}_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/include/json/json.hpp
        ${PROJECT_SOURCE_DIR}/src/json.cpp
    )
endif()
