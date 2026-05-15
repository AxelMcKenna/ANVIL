find_package(GTest CONFIG REQUIRED)

add_executable(anvil_tests
    smoke_test.cpp
    price_level_test.cpp
    order_book.cpp
    lobster_parser_test.cpp
)

target_link_libraries(anvil_tests PRIVATE
    anvil
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(anvil_tests)


