cmake_minimum_required(VERSION 3.15)
PROJECT(Tcp_System)

SET(CMAKE_BUILD_TYPE "Release")

SET(DIRECTORIES "")
add_subdirectory("Client")
add_subdirectory("Server")
