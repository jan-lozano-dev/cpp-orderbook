cmake_minimum_required(VERSION 3.20)

project(orderbook_tutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(orderbook_v1
    src/orderbook.cpp
    src/price_level.cpp
)

target_include_directories(orderbook_v1 PUBLIC
    include
)

if(MSVC)
    target_compile_options(orderbook_v1 PRIVATE /W4)
else()
    target_compile_options(orderbook_v1 PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(price_level_tests
    tests/price_level_tests.cpp
)
target_link_libraries(price_level_tests PRIVATE orderbook_v1)
add_test(NAME price_level_tests COMMAND price_level_tests)

add_executable(orderbook_tests
    tests/orderbook_tests.cpp
)
target_link_libraries(orderbook_tests PRIVATE orderbook_v1)
add_test(NAME orderbook_tests COMMAND orderbook_tests)
