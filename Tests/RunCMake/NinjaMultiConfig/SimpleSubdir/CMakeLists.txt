add_library(simpleobj OBJECT ../empty.c "$<$<CONFIG:Debug>:../empty_dbg.c>" "$<$<CONFIG:Release>:../empty_rel.c>")
add_library(simpleshared SHARED ../simplelib.c)

add_executable(simpleexe ../main.c $<TARGET_OBJECTS:simpleobj>)
target_link_libraries(simpleexe PRIVATE simpleshared)

add_executable(simpleexe2 ../main.c)
target_link_libraries(simpleexe2 PRIVATE simpleshared $<TARGET_OBJECTS:simpleobj>)
