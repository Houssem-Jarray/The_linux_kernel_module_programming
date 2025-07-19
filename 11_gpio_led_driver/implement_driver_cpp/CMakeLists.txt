cmake_minimum_required(VERSION 3.10)
project(ImplementDriverCpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(implement_driver_cpp main.cpp src/gpio.cpp)

target_include_directories(implement_driver_cpp PRIVATE ${CMAKE_SOURCE_DIR}/inc)