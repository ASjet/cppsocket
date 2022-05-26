cmake_minimum_required(VERSION 3.5)

project(cppsocket)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
    message("Environment: WIN32")
    add_library(
        ${PROJECT_NAME}
        src/socketIOwin.cpp
        src/socket.cpp
    )
    target_link_libraries( ${PROJECT_NAME} ws2_32 )
elseif(UNIX)
    message("Environment: UNIX")
    add_library(
        ${PROJECT_NAME}
        src/socketIOunix.cpp
        src/socket.cpp
    )
elseif(APPLE)
    message("Environment: APPLE")
    message(STATUS "Unsupported platform: Darwin.")
elseif(ANDROID)
    message("Environment: ANDROID")
    message(STATUS "Unsupported platform: Andriod.")
endif(WIN32)

add_executable(
    echos
    example/echos.cpp
)

add_executable(
    echoc
    example/echoc.cpp
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

add_compile_options(-Wall -g -O0 -fsanitize=address)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)