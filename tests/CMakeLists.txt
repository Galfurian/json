# -----------------------------------------------------------------------------
# @brief  : Test directory.
# @author : Enrico Fraccaroli
# @create : 03/01/2022
# @update : 03/01/2022
# -----------------------------------------------------------------------------

add_executable(json_test_data_types ${CMAKE_CURRENT_SOURCE_DIR}/src/test_data_types.cpp)
target_include_directories(json_test_data_types PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_test_data_types json)
target_compile_options(json_test_data_types PRIVATE ${JSON_COMPILE_OPTIONS})
add_test(NAME json_test_data_types_run COMMAND json_test_data_types)

add_executable(json_test_struct  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_struct.cpp)
target_include_directories(json_test_struct PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_test_struct json)
target_compile_options(json_test_struct PRIVATE ${JSON_COMPILE_OPTIONS})
add_test(NAME json_test_struct_run COMMAND json_test_struct)

add_executable(json_test_container ${CMAKE_CURRENT_SOURCE_DIR}/src/test_container.cpp)
target_include_directories(json_test_container PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_test_container json)
target_compile_options(json_test_container PRIVATE ${JSON_COMPILE_OPTIONS})
add_test(NAME json_test_container_run COMMAND json_test_container)

add_executable(json_test_friend ${CMAKE_CURRENT_SOURCE_DIR}/src/test_friend.cpp)
target_include_directories(json_test_friend PRIVATE ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(json_test_friend json)
target_compile_options(json_test_friend PRIVATE ${JSON_COMPILE_OPTIONS})
add_test(NAME json_test_friend_run COMMAND json_test_friend)
