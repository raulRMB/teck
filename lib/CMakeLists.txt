add_subdirectory(imgui)

add_library(tecklib INTERFACE)

target_include_directories(tecklib INTERFACE imgui)
target_link_libraries(tecklib INTERFACE imgui)
