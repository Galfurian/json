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

# -----------------------------------------------------------------------------
# Add options.
# -----------------------------------------------------------------------------
option(JSON_STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(JSON_WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)
option(JSON_BUILD_EXAMPLES "Build examples" OFF)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------
# Add the C++ library.
add_library(json INTERFACE)
add_library(json::json ALIAS json)

# Inlcude header directories.
target_include_directories(json INTERFACE ${PROJECT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# Set the compilation flags.
# -----------------------------------------------------------------------------
set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} -std=c++98")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} -D_CRT_SECURE_NO_WARNINGS")

    if(JSON_WARNINGS_AS_ERRORS)
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /WX")
    endif(JSON_WARNINGS_AS_ERRORS)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(JSON_WARNINGS_AS_ERRORS)
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} -Werror")
    endif(JSON_WARNINGS_AS_ERRORS)
endif()

if(JSON_STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /experimental:external")
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /external:I ${CMAKE_BINARY_DIR}")
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /external:anglebrackets")
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /external:W0")
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} /W4")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(JSON_COMPILE_OPTIONS "${JSON_COMPILE_OPTIONS} -Wall -Wextra -Wconversion -pedantic")
    endif()
endif(JSON_STRICT_WARNINGS)

string(REPLACE " " ";" JSON_COMPILE_OPTIONS ${JSON_COMPILE_OPTIONS})

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "JSON Library")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW NO)
    set(DOXYGEN_HTML_STYLESHEET ${PROJECT_SOURCE_DIR}/docs/doxygen.css)
    doxygen_add_docs(
        ${PROJECT_NAME}_documentation
        ${PROJECT_SOURCE_DIR}/include/json/json.hpp
    )
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(JSON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# -----------------------------------------------------------------------------
# Testing programs.
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)