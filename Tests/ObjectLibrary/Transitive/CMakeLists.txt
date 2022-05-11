add_library(FooStatic STATIC FooStatic.c)

add_library(FooObject1 OBJECT FooObject.c)
target_link_libraries(FooObject1 PRIVATE FooStatic)
add_executable(Transitive1 Transitive.c)
target_link_libraries(Transitive1 PRIVATE FooObject1)

add_library(FooObject2 OBJECT FooObject.c)
target_link_libraries(FooObject2 INTERFACE FooStatic)
add_executable(Transitive2 Transitive.c)
target_link_libraries(Transitive2 PRIVATE FooObject2)
