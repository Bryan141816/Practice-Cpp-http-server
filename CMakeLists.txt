cmake_minimum_required(VERSION 3.10.0)
project(Server VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Server main.cpp)

target_link_libraries(Server PRIVATE Ws2_32)
include(CTest)
enable_testing()

