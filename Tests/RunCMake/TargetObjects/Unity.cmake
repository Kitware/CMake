enable_language(C)

# Test transforming the set of object files provided by an object library.
set(CMAKE_UNITY_BUILD 1)

add_library(UnityObj1 OBJECT UnityObj1.c)
add_library(UnityObj2 OBJECT UnityObj2.c)

add_library(UnityObj2Iface INTERFACE)
target_link_libraries(UnityObj2Iface INTERFACE $<TARGET_OBJECTS:UnityObj2>)

add_executable(UnityMain UnityMain.c)
target_link_libraries(UnityMain PRIVATE UnityObj1 UnityObj2Iface)
