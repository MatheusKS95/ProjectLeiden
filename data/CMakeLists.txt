#TODO maybe bring back dynarray but a bit more generic than the one removed

add_library(data STATIC hashtable.c)
target_include_directories(data PUBLIC .)
target_link_libraries(data PUBLIC SDL3::SDL3)
