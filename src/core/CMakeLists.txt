cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
     ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
include (${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext EXTERNAL)

find_package(Vulkan)
find_package(glm)
find_package(glfw3)

# CPMAddPackage(
#   NAME entt
#   GITHUB_REPOSITORY skypjack/entt
#   GIT_TAG v3.13.2 
# )

add_library(tk_core ${CORE_SRC})
target_link_libraries(tk_core PUBLIC tk_ext Vulkan::Vulkan glm glfw)
target_include_directories(tk_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
