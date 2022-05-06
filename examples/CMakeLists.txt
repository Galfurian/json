# -----------------------------------------------------------------------------
# @brief  : Examples directory.
# @author : Enrico Fraccaroli
# @create : 03/01/2022
# @update : 03/01/2022
# -----------------------------------------------------------------------------

add_executable(${PROJECT_NAME}_example ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp)
target_include_directories(${PROJECT_NAME}_example PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_example json)