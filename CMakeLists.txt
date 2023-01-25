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
# OPTIONS
# -----------------------------------------------------------------------------

option(JSON_BUILD_EXAMPLES "Build examples" OFF)
option(JSON_BUILD_TESTS "Build tests" OFF)
option(JSON_STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(JSON_WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)

# -----------------------------------------------------------------------------
# COMPILATION FLAGS
# -----------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} -D_CRT_SECURE_NO_WARNINGS)

    if(JSON_WARNINGS_AS_ERRORS)
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /WX)
    endif(JSON_WARNINGS_AS_ERRORS)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(JSON_WARNINGS_AS_ERRORS)
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} -Werror)
    endif(JSON_WARNINGS_AS_ERRORS)
endif()

if(JSON_STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /experimental:external)
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /external:I ${CMAKE_BINARY_DIR})
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /external:anglebrackets)
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /external:W0)
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS} -Wall -Wextra -Wconversion -pedantic)
    endif()
endif(JSON_STRICT_WARNINGS)

# -----------------------------------------------------------------------------
# DIPENDENCIES
# -----------------------------------------------------------------------------

# We need this in order to import external projects.
include(FetchContent)

# Retrieve the source.
message(STATUS "Retrieving `ordered_map`...")
FetchContent_Declare(ordered_map
    GIT_REPOSITORY "https://github.com/Galfurian/cpp_ordered_map.git"
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(ordered_map)
if(NOT ordered_map_POPULATED)
    # Ensures the named dependencies have been populated.
    FetchContent_MakeAvailable(ordered_map)
    # Hide fetchcontent variables, otherwise with ccmake it's a mess.
    mark_as_advanced(FORCE
        FETCHCONTENT_QUIET FETCHCONTENT_BASE_DIR FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_UPDATES_DISCONNECTED
        FETCHCONTENT_UPDATES_DISCONNECTED_ORDERED_MAP FETCHCONTENT_SOURCE_DIR_ORDERED_MAP
    )
endif(NOT ordered_map_POPULATED)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# Inlcude header directories and set the library.
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include ${ordered_map_SOURCE_DIR}/include)
# Set compilation flags.
target_compile_options(${PROJECT_NAME} INTERFACE ${JSON_COMPILE_OPTIONS})

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(JSON_BUILD_EXAMPLES)

    # = FETCH =================================================================

    # We need this in order to import external projects.
    include(FetchContent)

    # Retrieve the stopwatch.
    message(STATUS "Retrieving `stopwatch`...")
    FetchContent_Declare(stopwatch
        GIT_REPOSITORY "https://github.com/Galfurian/stopwatch.git"
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )

    FetchContent_GetProperties(stopwatch)
    if(NOT stopwatch_POPULATED)
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
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_example PUBLIC ${JSON_COMPILE_OPTIONS})
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_example PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_example PUBLIC ${PROJECT_NAME})
    
    # Add the example.
    add_executable(${PROJECT_NAME}_run_on_file examples/run_on_file.cpp)
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_run_on_file PUBLIC ${JSON_COMPILE_OPTIONS})
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_run_on_file PUBLIC ${stopwatch_SOURCE_DIR}/include)
    # Set the linked libraries.
    target_link_libraries(${PROJECT_NAME}_run_on_file PUBLIC ${PROJECT_NAME})
    
endif()

# -----------------------------------------------------------------------------
# TESTS
# -----------------------------------------------------------------------------

if(JSON_BUILD_TESTS)

    # CMake has support for adding tests to a project.
    enable_testing()

    # -------------------------------------
    # TEST
    # -------------------------------------
    # Add the test.
    add_executable(${PROJECT_NAME}_test_data_types ${PROJECT_SOURCE_DIR}/tests/src/test_data_types.cpp)
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_test_data_types PUBLIC ${JSON_COMPILE_OPTIONS})
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
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_test_struct PUBLIC ${JSON_COMPILE_OPTIONS})
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
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_test_container PUBLIC ${JSON_COMPILE_OPTIONS})
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
    # Set compilation flags.
    target_compile_options(${PROJECT_NAME}_test_friend PUBLIC ${JSON_COMPILE_OPTIONS})
    # Inlcude header directories.
    target_include_directories(${PROJECT_NAME}_test_friend PUBLIC ${PROJECT_SOURCE_DIR}/tests/include)
    # Liking for the test.
    target_link_libraries(${PROJECT_NAME}_test_friend ${PROJECT_NAME})
    # Add the test.
    add_test(${PROJECT_NAME}_test_friend_run ${PROJECT_NAME}_test_friend)

endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

find_package(Doxygen)

if(DOXYGEN_FOUND)
    
    message(STATUS "Retrieving `doxygen-awesome-css`...")

    # = RETIVAL ===============================================================
    # Include fetch content.
    include(FetchContent)
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
        # Ensures the named dependencies have been populated.
        FetchContent_MakeAvailable(doxygenawesome)
        # Hide fetchcontent variables, otherwise with ccmake it's a mess.
        mark_as_advanced(FORCE
            FETCHCONTENT_QUIET FETCHCONTENT_BASE_DIR FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_UPDATES_DISCONNECTED
            FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
        )
    endif()

    # = CUSTOMIZATION =========================================================
    set(DOXYGEN_PROJECT_NAME "JSON Parser/Writer Library")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
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
