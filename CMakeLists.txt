# -----------------------------------------------------------------------------
# @brief  : CmakeFile for building the executable.
# @author : Enrico Fraccaroli
# @create : 02/02/2018
# @update : 20/08/2019
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Set the minimum CMake version.
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8...3.22)

# Set the project name.
project(json CXX)

# Set the default build type to Debug.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

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
# OPTIONS
# -----------------------------------------------------------------------------

option(${PROJECT_NAME}_BUILD_EXAMPLES "Build examples" OFF)
option(${PROJECT_NAME}_BUILD_TESTS "Build tests" OFF)
option(${PROJECT_NAME}_STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(${PROJECT_NAME}_WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)

# -----------------------------------------------------------------------------
# DEPENDENCY (SYSTEM LIBRARIES)
# -----------------------------------------------------------------------------

find_package(Doxygen)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# Inlcude header directories and set the library.
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# COMPILATION FLAGS
# -----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(${PROJECT_NAME} INTERFACE _CRT_SECURE_NO_WARNINGS)
    if(${PROJECT_NAME}_WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(${PROJECT_NAME}_WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} INTERFACE -Werror)
    endif()
endif()

if(${PROJECT_NAME}_STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(${PROJECT_NAME} INTERFACE /experimental:external)
        target_compile_options(${PROJECT_NAME} INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(${PROJECT_NAME} INTERFACE /external:anglebrackets)
        target_compile_options(${PROJECT_NAME} INTERFACE /external:W0)

        target_compile_options(${PROJECT_NAME} INTERFACE /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# DIPENDENCIES
# -----------------------------------------------------------------------------

# Retrieve the source.
FetchContent_Declare(ordered_map
    GIT_REPOSITORY "https://github.com/Galfurian/cpp_ordered_map.git"
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# Retrieve the properties related to the content.
FetchContent_GetProperties(ordered_map)
# If not populated, make the content available.
if(NOT ordered_map_POPULATED)
    message(STATUS "Retrieving `ordered_map`...")
    # Ensures the named dependencies have been populated.
    FetchContent_MakeAvailable(ordered_map)
    # Hide fetchcontent variables, otherwise with ccmake it's a mess.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_ORDERED_MAP
        FETCHCONTENT_SOURCE_DIR_ORDERED_MAP
    )
endif(NOT ordered_map_POPULATED)
# Inlcude header directories and set the library.
target_include_directories(${PROJECT_NAME} INTERFACE ${ordered_map_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(${PROJECT_NAME}_BUILD_EXAMPLES)

    # Retrieve the stopwatch.
    FetchContent_Declare(stopwatch
        GIT_REPOSITORY "https://github.com/Galfurian/stopwatch.git"
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    # Retrieve the properties related to the content.
    FetchContent_GetProperties(stopwatch)
    # If not populated, make the content available.
    if(NOT stopwatch_POPULATED)
        message(STATUS "Retrieving `stopwatch`...")
        # Ensures the named dependencies have been populated.
        FetchContent_MakeAvailable(stopwatch)
        # Hide fetchcontent variables, otherwise with ccmake it's a mess.
        mark_as_advanced(FORCE
            FETCHCONTENT_QUIET FETCHCONTENT_BASE_DIR FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_UPDATES_DISCONNECTED
            FETCHCONTENT_UPDATES_DISCONNECTED_STOPWATCH FETCHCONTENT_SOURCE_DIR_STOPWATCH
        )
    endif(NOT stopwatch_POPULATED)

    # = TARGETS ===============================================================

    # Add the example.
    add_executable(${PROJECT_NAME}_example examples/example.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_example PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_example PUBLIC ${PROJECT_NAME})
    # Set compiler flags.
    target_compile_features(${PROJECT_NAME}_example PUBLIC cxx_std_98)
    
    # Add the example.
    add_executable(${PROJECT_NAME}_run_on_file examples/run_on_file.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_run_on_file PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_run_on_file PUBLIC ${PROJECT_NAME})
    # Set compiler flags.
    target_compile_features(${PROJECT_NAME}_run_on_file PUBLIC cxx_std_98)
    
endif(${PROJECT_NAME}_BUILD_EXAMPLES)

# -----------------------------------------------------------------------------
# TESTS
# -----------------------------------------------------------------------------

if(${PROJECT_NAME}_BUILD_TESTS)

    # CMake has support for adding tests to a project.
    enable_testing()

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_data_types ${PROJECT_SOURCE_DIR}/tests/src/test_data_types.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_data_types PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_data_types ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_data_types_run ${PROJECT_NAME}_test_data_types)

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_struct ${PROJECT_SOURCE_DIR}/tests/src/test_struct.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_struct PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_struct ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_struct_run ${PROJECT_NAME}_test_struct)

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_container ${PROJECT_SOURCE_DIR}/tests/src/test_container.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_container PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_container ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_container_run ${PROJECT_NAME}_test_container)

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_friend ${PROJECT_SOURCE_DIR}/tests/src/test_friend.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_friend PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_friend ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_friend_run ${PROJECT_NAME}_test_friend)

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_order ${PROJECT_SOURCE_DIR}/tests/src/test_order.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_order PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_order ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_order_run ${PROJECT_NAME}_test_order)

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_error ${PROJECT_SOURCE_DIR}/tests/src/test_error.cpp)
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_error PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_error ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_error_run ${PROJECT_NAME}_test_error)

endif(${PROJECT_NAME}_BUILD_TESTS)

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # Record the options that describe how to populate the specified content.
    FetchContent_Declare(
        doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG 4cd62308d825fe0396d2f66ffbab45d0e247724c # 2.0.3
    )
    # Retrieve the properties related to the content.
    FetchContent_GetProperties(doxygenawesome)
    # If not populated, make the content available.
    if(NOT doxygenawesome_POPULATED)
        message(STATUS "Retrieving `doxygen-awesome-css`...")
        # Ensures the named dependencies have been populated.
        FetchContent_MakeAvailable(doxygenawesome)
        # Hide fetchcontent variables, otherwise with ccmake it's a mess.
        mark_as_advanced(FORCE
            FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
            FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
        )
    endif()

    # = CUSTOMIZATION =========================================================
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
    )
endif()
