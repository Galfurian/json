# -----------------------------------------------------------------------------
# @brief  : Test directory.
# @author : Enrico Fraccaroli
# @create : 03/01/2022
# @update : 03/01/2022
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
add_executable(test_data_types test_data_types.cpp)
target_include_directories(test_data_types PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_data_types json)
add_test(NAME run_test_data_types COMMAND test_data_types)

add_executable(test_pointers test_pointers.cpp)
target_include_directories(test_pointers PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_pointers json)
add_test(NAME run_test_pointers COMMAND test_pointers)
