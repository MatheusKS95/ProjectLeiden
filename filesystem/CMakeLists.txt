# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

# The rockport-filesystem target
add_library(filesystem STATIC fileio.c ini.c)
target_include_directories(filesystem PUBLIC .)
target_link_libraries(filesystem PUBLIC SDL3::SDL3)
