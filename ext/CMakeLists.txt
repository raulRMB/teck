add_subdirectory(imgui)

add_library(tk_ext INTERFACE)

target_link_libraries(tk_ext INTERFACE imgui)
target_include_directories(tk_ext INTERFACE imgui)
