cmake_minimum_required(VERSION 3.5)

project(cppsocket)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_library(
    ${PROJECT_NAME}
    src/socket.cpp
)

add_subdirectory(
    zlib
)

target_link_libraries(
    ${PROJECT_NAME}
    pthread
    zlib
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
