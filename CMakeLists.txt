cmake_minimum_required(VERSION 3.7.0)

project(bluetooth)

set (CMAKE_C_FLAGS "-D_GNU_SOURCE -Wall -std=c99")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BUILD_TYPE Debug)

add_executable(bluetooth_server "")

target_link_libraries(bluetooth_server PUBLIC hardware bluetooth)

target_sources(bluetooth_server
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/bluetooth_server.c
  )

install(TARGETS bluetooth_server DESTINATION bin)
file(COPY config/kill_process.sh DESTINATION bin)
file(COPY scripts/sdp.sh DESTINATION bin)