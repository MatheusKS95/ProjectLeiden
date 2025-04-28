add_library(input STATIC input.c)
target_include_directories(input PUBLIC .)
target_link_libraries(input PUBLIC SDL3::SDL3)
