cmake_minimum_required(VERSION 3.10)

project(jet)

file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/cpm.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include (${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json)
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
  file(COPY ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
endif()

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/rec/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rec/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/rec/)

file(GLOB_RECURSE SRC src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})

add_subdirectory(lib)

if(DEFINED _WIN32)
  CPMAddPackage(
          NAME glm
          GITHUB_REPOSITORY g-truc/glm
          GIT_TAG 0.9.9.7
  )

  CPMAddPackage(
          NAME glfw
          GITHUB_REPOSITORY glfw/glfw
          GIT_TAG 3.4
  )

  target_include_directories(${PROJECT_NAME} PUBLIC C:/VulkanSDK/1.3.283.0/Include ${CMAKE_CURRENT_SOURCE_DIR}/core/)
  target_link_libraries(${PROJECT_NAME} glm glfw C:/VulkanSDK/1.3.283.0/Lib/vulkan-1.lib)

else()

  find_package(glfw3 REQUIRED)
  find_package(Vulkan REQUIRED)
  find_package(glm REQUIRED)
  find_package(GameNetworkingSockets REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan glm jetlib GameNetworkingSockets)
  target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ jetlib)

endif()


