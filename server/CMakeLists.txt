cmake_minimum_required(VERSION 3.10)

project(server C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    main.c
    server.c
    socket.c
    signal_handler.c
    cal.c
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server
    pthread
    m
)
