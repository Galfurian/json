# -----------------------------------------------------------------------------
# @brief  : Examples directory.
# @author : Enrico Fraccaroli
# @create : 03/01/2022
# @update : 03/01/2022
# -----------------------------------------------------------------------------

add_executable(json_example ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp)
target_include_directories(json_example PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_example json)
target_compile_options(json_example PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(json_run_on_file ${CMAKE_CURRENT_SOURCE_DIR}/run_on_file.cpp)
target_include_directories(json_run_on_file PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_run_on_file json)
target_compile_options(json_run_on_file PRIVATE ${JSON_COMPILE_OPTIONS})