enable_language(C)

add_executable(exe_1 main.c)

add_library(static STATIC obj1.c)
add_library(shared_1 SHARED obj5.c)
add_library(shared_2 SHARED obj3.c)
add_library(shared_3 SHARED obj4.c)

add_executable(exe_2 main.c)
add_library(iface INTERFACE)

target_link_libraries(exe_1 PRIVATE static shared_1)
target_link_libraries(static PRIVATE shared_2)
target_link_libraries(shared_1 PRIVATE shared_3)

target_link_libraries(exe_2 PRIVATE iface)
target_link_libraries(iface INTERFACE static shared_1)

install(
  TARGETS exe_1 RUNTIME_DEPENDENCY_TARGETS
  COMPONENT exc
  LIBRARY DESTINATION lib/first
  RUNTIME DESTINATION bin/first)
install(
  TARGETS exe_2 RUNTIME_DEPENDENCY_TARGETS
  COMPONENT exc
  LIBRARY DESTINATION lib/second
  RUNTIME DESTINATION bin/second)
