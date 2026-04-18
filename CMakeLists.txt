cmake_minimum_required(VERSION 3.15)
project(tcp_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp)
target_link_libraries(server pthread)