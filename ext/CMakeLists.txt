add_subdirectory(imgui)

add_library(teck_ext INTERFACE)

target_include_directories(teck_ext INTERFACE imgui)
target_link_libraries(teck_ext INTERFACE imgui)
