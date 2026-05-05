cmake_minimum_required(VERSION 3.18.4)
project(super_bulles C)

set(CMAKE_C_STANDARD 11)

add_executable(super_bulles
        interface_graphique.c
        interface_graphique.h)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -L/usr/local/lib -lalleg -lm")
elseif(UNIX)
    target_link_libraries(super_bulles alleg m)
elseif(WIN32)
    target_link_libraries(super_bulles -lmingw32 -lalleg44)
endif()