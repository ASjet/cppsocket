cmake_minimum_required(VERSION 3.5)

project(socket)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

link_libraries(
    pthread
)

add_executable(
    client
    src/client.cpp
    src/socket.cpp
)

add_executable(
    server
    src/server.cpp
    src/socket.cpp
)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
add_compile_options(-O0 -g)