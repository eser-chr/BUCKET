cmake_minimum_required(VERSION 3.16)
enable_testing()

add_executable(testA testA.cpp)
add_executable(test_concepts test_concepts.cpp)

# Link bucket library and include doctest
target_link_libraries(testA PRIVATE bucket)
target_link_libraries(test_concepts PRIVATE bucket)

# Make sure include path is inherited
target_include_directories(testA PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_include_directories(test_concepts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_test(NAME testA COMMAND testA)
add_test(NAME test_concepts COMMAND test_concepts)
