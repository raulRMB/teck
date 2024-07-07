cmake_minimum_required(VERSION 3.28)

project(client)

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/rec/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rec/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/rec/)

set(CMAKE_BUILD_TYPE Debug)

set(TK_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(MODULES_DIR ${TK_MAIN_SRC}/modules)

set(MODULE_LIST networking)

foreach(MOD IN LISTS MODULE_LIST)
  add_subdirectory(${MODULES_DIR}/${MOD} TK_MODULES)
endforeach()

# Get all directories in the ext folder
set(TK_EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../ext/)
file(GLOB CHILDREN RELATIVE ${TK_EXT_DIR} ${TK_EXT_DIR}/*)
foreach(CHILD ${CHILDREN})
  if (IS_DIRECTORY ${TK_EXT_DIR}/${CHILD})
    list(APPEND TK_EXT ${TK_EXT_DIR}${CHILD})
  endif()
endforeach()

add_subdirectory(${TK_MAIN_SRC}/core TK_CORE)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC networking tk_core)
target_include_directories(${PROJECT_NAME} PUBLIC ${MODULES_DIR} ${TK_MAIN_SRC} ${TK_EXT})
