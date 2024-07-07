cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext EXTERNAL)

find_package(Vulkan)
find_package(glm)
find_package(glfw3)

add_library(tk_core ${CORE_SRC})
target_link_libraries(tk_core PUBLIC teck_ext Vulkan::Vulkan glm glfw)
target_include_directories(tk_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
