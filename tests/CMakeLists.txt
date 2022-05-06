# -----------------------------------------------------------------------------
# @brief  : Test directory.
# @author : Enrico Fraccaroli
# @create : 03/01/2022
# @update : 03/01/2022
# -----------------------------------------------------------------------------

add_executable(${PROJECT_NAME}_test_data_types ${CMAKE_CURRENT_SOURCE_DIR}/src/test_data_types.cpp)
target_include_directories(${PROJECT_NAME}_test_data_types PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_test_data_types json)
add_test(NAME ${PROJECT_NAME}_test_data_types_run COMMAND ${PROJECT_NAME}_test_data_types)

add_executable(${PROJECT_NAME}_test_struct  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_struct.cpp)
target_include_directories(${PROJECT_NAME}_test_struct PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_test_struct json)
add_test(NAME ${PROJECT_NAME}_test_struct_run COMMAND ${PROJECT_NAME}_test_struct)

add_executable(${PROJECT_NAME}_test_container ${CMAKE_CURRENT_SOURCE_DIR}/src/test_container.cpp)
target_include_directories(${PROJECT_NAME}_test_container PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_test_container json)
add_test(NAME ${PROJECT_NAME}_test_container_run COMMAND ${PROJECT_NAME}_test_container)