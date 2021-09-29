cmake_minimum_required(VERSION 3.5)

project(HeLeCheLun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(HeLeCheLun main.cpp)
target_link_libraries(HeLeCheLun PUBLIC FreeImageLite)
