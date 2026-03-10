cmake_minimum_required(VERSION 3.20)
project(3DRenderer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED)

add_executable(renderer src/main.c)
target_link_libraries(renderer PRIVATE SDL3::SDL3)
