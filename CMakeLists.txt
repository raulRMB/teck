cmake_minimum_required(VERSION 3.10)

project(jet)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json)
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
  file(COPY ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
endif()

file(GLOB_RECURSE SRC src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/)

# target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc" "C:/VulkanSDK/1.3.236.0/Include")
# target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/glfw3dll.lib" "C:/VulkanSDK/1.3.236.0/Lib/vulkan-1.lib")
