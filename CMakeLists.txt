cmake_minimum_required(VERSION 3.5)

project(cppsocket)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_library(
    ${PROJECT_NAME}
    src/Socket.cpp
    src/http.cpp
    src/socketIOunix.cpp
    src/socketIOwin.cpp
)

add_subdirectory(
    zlib
)

target_link_libraries(
    ${PROJECT_NAME}
    pthread
    zlib
)

add_executable(
    echos
    example/server.cpp
)

add_executable(
    echoc
    example/client.cpp
)

target_link_libraries(
    echos
    ${PROJECT_NAME}
)

target_link_libraries(
    echoc
    ${PROJECT_NAME}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
