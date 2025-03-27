# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

add_library(input STATIC input.c)
target_include_directories(input PUBLIC .)
target_link_libraries(input PUBLIC SDL3::SDL3)
