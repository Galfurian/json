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
option(STRICT_TYPE_CHECK "Enable strict json type checking at runtime" OFF)
option(STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

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
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(json INTERFACE _CRT_SECURE_NO_WARNINGS)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(json INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        target_compile_options(json INTERFACE -Werror)
    endif()
endif()

if(STRICT_TYPE_CHECK)
    # Pass the define to the code.
    target_compile_definitions(json INTERFACE STRICT_TYPE_CHECK)
endif()

if(STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(json INTERFACE /experimental:external)
        target_compile_options(json INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(json INTERFACE /external:anglebrackets)
        target_compile_options(json INTERFACE /external:W0)

        target_compile_options(json INTERFACE /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(json INTERFACE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR})
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)

    # Replace variables inside @@ with the current values.
    set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/docs/doxyfile.in)
    set(DOXYFILE_OUT ${PROJECT_BINARY_DIR}/doxyfile.out)
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    # Copy the files needd by the documentation.
    set(DOXYFILE_CSS ${PROJECT_SOURCE_DIR}/docs/doxygen.css)
    set(DOXYFILE_HEADER ${PROJECT_SOURCE_DIR}/docs/header.html)
    set(DOXYFILE_FOOTER ${PROJECT_SOURCE_DIR}/docs/footer.html)

    # Doxygen won't create this for us.
    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

    add_custom_command(
        OUTPUT ${DOXYGEN_INDEX_FILE}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_custom_target(
        ${PROJECT_NAME}_documentation
        DEPENDS
        ${DOXYGEN_INDEX_FILE}
        ${DOXYFILE_IN}
        ${DOXYFILE_CSS}
        ${DOXYFILE_HEADER}
        ${DOXYFILE_FOOTER}
    )
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# -----------------------------------------------------------------------------
# Testing programs.
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)