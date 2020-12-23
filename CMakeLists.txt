cmake_minimum_required(VERSION 3.7.0)

project(bluetooth)

set (CMAKE_C_FLAGS "-D_GNU_SOURCE -Wall -std=c99")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BUILD_TYPE Debug)

add_executable(led_subscriber "")
target_link_libraries(led_subscriber PUBLIC hardware paho-mqtt3c)
target_sources(led_subscriber
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/led_subscriber.c
  )
install(TARGETS led_subscriber DESTINATION bin)

add_executable(button_publish "")
target_link_libraries(button_publish PUBLIC hardware paho-mqtt3c)
target_sources(button_publish
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/button_publish.c
  )
install(TARGETS button_publish DESTINATION bin)

file(COPY properties/mqtt_properties.json DESTINATION bin)