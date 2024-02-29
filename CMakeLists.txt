cmake_minimum_required(VERSION 3.5)
PROJECT(Tcp_System)

SET(CMAKE_BUILD_TYPE "Release")

SET(DIRECTORIES "")
add_subdirectory("Client")
add_subdirectory("Server")

if(${INSTALL_SAMPLES})
    INSTALL(TARGETS ${DIRECTORIES} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()