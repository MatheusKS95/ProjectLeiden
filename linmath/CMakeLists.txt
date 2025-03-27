add_library(linmath STATIC linmath.c)
target_include_directories(linmath PUBLIC .)
target_link_libraries(linmath PUBLIC SDL3::SDL3)