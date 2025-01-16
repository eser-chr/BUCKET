cmake_minimum_required(3.16)
enable_testing()

add_executable(testA testA.cpp)
target_include_directories(${SOURCE_DIR}/include)
target_link_libraries(bucket)
add_test(NAME testA COMMAND testA)
