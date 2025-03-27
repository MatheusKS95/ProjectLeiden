# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'SDL3' exist.

# The rockport-util target
add_library(data STATIC dynarray.c hashtable.c)
target_include_directories(data PUBLIC .)
target_link_libraries(data PUBLIC SDL3::SDL3)
