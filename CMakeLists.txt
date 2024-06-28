cmake_minimum_required(VERSION 3.25)
project(beast-service)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.80 REQUIRED)

add_executable(beast-service main.cpp)
