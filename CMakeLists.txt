cmake_minimum_required(VERSION 3.30)
project(dst2svg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dst2svg
    src/main.cpp
    src/dst.cpp
)

target_include_directories(dst2svg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)