cmake_minimum_required(VERSION 3.10)

project(Lab2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -g -pthread)

add_library(hex hex.c)
add_executable(main main.c)
target_link_libraries(main pthread hex)