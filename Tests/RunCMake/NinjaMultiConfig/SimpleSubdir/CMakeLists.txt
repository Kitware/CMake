add_library(simpleobj OBJECT ../empty.c)
add_executable(simpleexe ../main.c $<TARGET_OBJECTS:simpleobj>)
add_library(simpleshared SHARED ../simplelib.c)
target_link_libraries(simpleexe PRIVATE simpleshared)
