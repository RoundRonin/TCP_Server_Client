CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(Tcp_Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_BUILD_TYPE "Release")

include(FetchContent)

set(JSONCPP_URL "https://github.com/open-source-parsers/jsoncpp/archive/1.9.5.tar.gz")

FetchContent_Declare(
    jsoncpp
    URL ${JSONCPP_URL}
)
FetchContent_MakeAvailable(jsoncpp)

ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    jsoncpp_lib
    )
