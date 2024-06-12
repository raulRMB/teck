add_subdirectory(imgui)

add_library(jetlib INTERFACE)

target_include_directories(jetlib INTERFACE imgui)
target_link_libraries(jetlib INTERFACE imgui)
