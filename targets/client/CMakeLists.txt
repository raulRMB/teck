cmake_minimum_required(VERSION 3.28)

project(client)

file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
     ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
include (${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/rec/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rec/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/rec/)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json)
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
  file(COPY ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_BUILD_TYPE Debug)

set(TK_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/)

file(GLOB_RECURSE TK_CORE ${TK_MAIN_SRC}/core/*.cpp)
file(GLOB_RECURSE TK_SHARED_MODULES ${TK_MAIN_SRC}/modules/shared/*.cpp)
file(GLOB_RECURSE TK_CLIENT_MODULES ${TK_MAIN_SRC}/modules/client/*.cpp)

add_subdirectory(${TK_MAIN_SRC}/core/ TK_CORE)

find_package(GameNetworkingSockets REQUIRED)

add_executable(${PROJECT_NAME} main.cpp ${TK_CLIENT_MODULES} ${TK_SHARED_MODULES})
target_link_libraries(${PROJECT_NAME} PUBLIC GameNetworkingSockets tk_core)
target_include_directories(${PROJECT_NAME} PUBLIC ${TK_MAIN_SRC})
