# Let's write this by hand slowly so I refresh my memory :)

# 1. DECLARE PROJECT AND LANGUAGE
cmake_minimum_required(VERSION 3.20)

project(orderbook_terminal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Prevent falling back to older standard.
set(CMAKE_CXX_EXTENSIONS OFF) #Select standard C++ rather than compiler specific dialect

# Locate qt widgets (Now Qt6::Widgets is available)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include existing engine
enable_testing()

add_subdirectory(src/engine)

#Define application
add_executable(orderbook_terminal
    src/main.cpp
    src/ui/MainWindow.cpp
)

target_link_libraries(orderbook_terminal PRIVATE
    Qt6::Widgets
    orderbook_v1
)