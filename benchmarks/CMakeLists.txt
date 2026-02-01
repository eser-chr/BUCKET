add_executable(bucket_bench
    main.cpp
)

target_link_libraries(bucket_bench
    PRIVATE bucket
)

target_compile_features(bucket_bench PRIVATE cxx_std_20)

target_compile_options(bucket_bench PRIVATE
    -O3
    -DNDEBUG
)

set_target_properties(bucket_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmarks
)
