cmake_minimum_required(VERSION 3.21)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED) # example

add_executable(MyApp src/main.cpp)

target_link_libraries(MyApp PRIVATE fmt::fmt) # example
