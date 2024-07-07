cmake_minimum_required(VERSION 3.18)

file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(CMAKE_CXX_STANDARD 23)

get_filename_component(MOD_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

find_package(GameNetworkingSockets)

add_library(${MOD_NAME} ${SRC})
target_link_libraries(${MOD_NAME} INTERFACE GameNetworkingSockets)
target_include_directories(${MOD_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../)
