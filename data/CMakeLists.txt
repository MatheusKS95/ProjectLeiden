add_library(data STATIC dynarray.c hashtable.c)
target_include_directories(data PUBLIC .)
target_link_libraries(data PUBLIC SDL3::SDL3)
