cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(mc-proxy)

include(FetchContent)

FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.0.0"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)

FetchContent_Declare(ExpectedLib
    GIT_REPOSITORY "https://github.com/TartanLlama/expected"
    GIT_TAG "v1.1.0"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(ExpectedLib)

add_executable(mc-proxy
    src/main.cc
    src/shutdown_handler.cc
    src/platform/linux/linux.cc
    src/platform/linux/io.cc
    src/platform/linux/net.cc
    src/platform/linux/signal.cc)

target_link_libraries(mc-proxy PRIVATE Microsoft.GSL::GSL)
target_link_libraries(mc-proxy PRIVATE tl::expected)
