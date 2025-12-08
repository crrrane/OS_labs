cmake_minimum_required(VERSION 3.10)

project(Lab2_Dynamic)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -g -fPIC)

add_library(lib1 SHARED lib1.c)
add_library(lib2 SHARED lib2.c)

target_link_libraries(lib1 m)
target_link_libraries(lib2 m)

add_executable(static static.c lib1.c)
target_link_libraries(static m)

add_executable(dynamic dynamic.c)
target_link_libraries(dynamic dl m)