find_package(benchmark CONFIG REQUIRED)

add_executable(anvil_bench
    order_book_bench.cpp
)

target_link_libraries(anvil_bench PRIVATE
    anvil
    benchmark::benchmark
    benchmark::benchmark_main
)
