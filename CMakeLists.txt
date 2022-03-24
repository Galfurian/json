# -----------------------------------------------------------------------------
# @brief  : CmakeFile for building the executable.
# @author : Enrico Fraccaroli
# @create : 02/02/2018
# @update : 20/08/2019
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Set the minimum CMake version.
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1...3.22)

# Set the project name.
project(json CXX)


# -----------------------------------------------------------------------------
# Set the compilation flags.
# -----------------------------------------------------------------------------

# Only meddle with non-required compilation flags,
# if we are the top level project to ensure consistency
# with parent project otherwise
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)

    # Set the default build type to Debug.
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Debug' as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    endif()

    # Set C++ Standard.
    set(CMAKE_CXX_STANDARD 20)

    # Set flags that affect compilation
    if(MSVC)
        # Enable standards-conformance mode for MSVC compilers that support this flag
        # (Visual C++ 2017 and later).
        add_compile_options(/permissive-)
    else()
        add_compile_options(
            $<$<CONFIG:Debug>:-g3>
            $<$<CONFIG:Debug>:-ggdb>
            $<$<CONFIG:Debug>:-O0>
        )
    endif()

    # Set Warning flags
    if(MSVC)
        add_compile_options(/W4)
    else()
        add_compile_options(-Wall)
        add_compile_options(-Werror)
        add_compile_options(-pedantic)
        add_compile_options(-pedantic-errors)
    endif()

endif() # hasParent

# -----------------------------------------------------------------------------
# C++ Library with Python wrapping.
# -----------------------------------------------------------------------------
# Add the python wrapped library.
add_library(${PROJECT_NAME} src/json/json.cpp)
# Inlcude header directories.
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# Testing programs.
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(test EXCLUDE_FROM_ALL)