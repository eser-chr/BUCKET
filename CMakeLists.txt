cmake_minimum_required(VERSION 3.16)
project(bucket LANGUAGES C CXX)
enable_testing()

# Define header-only interface library
add_library(bucket INTERFACE)


target_include_directories(bucket INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(bucket INTERFACE cxx_std_20)

# Optional: Install headers and export targets
if(BUCKET_INSTALL)
    install(TARGETS bucket EXPORT bucket_Targets)
    install(DIRECTORY include/ DESTINATION include)
endif()

export(TARGETS bucket FILE bucketConfig.cmake)

# Add tests
add_subdirectory(tests)
add_subdirectory(benchmarks)
