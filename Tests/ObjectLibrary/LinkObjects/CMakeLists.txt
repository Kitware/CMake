add_executable(LinkObjects main.c)

# Link TARGET_OBJECTS through LINK_LIBRARIES.
add_library(LinkObjectsAObj OBJECT a_obj.c)
add_library(LinkObjectsADep STATIC a_dep.c)
target_compile_definitions(LinkObjectsAObj PUBLIC OBJA)
target_link_libraries(LinkObjects PRIVATE LinkObjectsADep $<TARGET_OBJECTS:LinkObjectsAObj>)

# Link TARGET_OBJECTS through INTERFACE_LINK_LIBRARIES with usage requirements.
add_library(LinkObjectsB INTERFACE)
add_library(LinkObjectsBObj OBJECT b_obj.c)
add_library(LinkObjectsBDep STATIC b_dep.c)
target_compile_definitions(LinkObjectsBObj PUBLIC OBJB)
target_link_libraries(LinkObjectsB INTERFACE LinkObjectsBObj $<TARGET_OBJECTS:LinkObjectsBObj>)
target_link_libraries(LinkObjectsBObj PRIVATE LinkObjectsBDep)
target_link_libraries(LinkObjects PRIVATE LinkObjectsB)

# Link TARGET_OBJECTS through INTERFACE_LINK_LIBRARIES without usage requirements.
add_library(LinkObjectsC INTERFACE)
add_library(LinkObjectsCObj OBJECT c_obj.c)
add_library(LinkObjectsCDep STATIC c_dep.c)
target_compile_definitions(LinkObjectsCObj PUBLIC OBJC)
target_link_libraries(LinkObjectsC INTERFACE LinkObjectsCDep $<TARGET_OBJECTS:LinkObjectsCObj>)
target_link_libraries(LinkObjectsCObj PRIVATE LinkObjectsCDep)
target_link_libraries(LinkObjects PRIVATE LinkObjectsC)

# Link TARGET_OBJECTS through both LINK_LIBRARIES and INTERFACE_LINK_LIBRARIES, deduplicated.
add_library(LinkObjectsD INTERFACE)
add_library(LinkObjectsDObj OBJECT d_obj.c)
add_library(LinkObjectsDDep STATIC d_dep.c)
target_compile_definitions(LinkObjectsDObj PUBLIC OBJD)
target_link_libraries(LinkObjectsD INTERFACE LinkObjectsDObj $<TARGET_OBJECTS:LinkObjectsDObj>)
target_link_libraries(LinkObjectsDObj PRIVATE LinkObjectsDDep)
target_link_libraries(LinkObjects PRIVATE $<TARGET_OBJECTS:LinkObjectsDObj> LinkObjectsD)

# Link TARGET_OBJECTS through STATIC library private dependency.
add_library(LinkObjectsE INTERFACE)
add_library(LinkObjectsEObj OBJECT e_obj.c)
add_library(LinkObjectsEDep STATIC e_dep.c)
add_library(LinkObjectsEStatic STATIC e_lib.c)
target_compile_definitions(LinkObjectsEObj PUBLIC OBJE)
target_link_libraries(LinkObjectsE INTERFACE LinkObjectsEObj $<TARGET_OBJECTS:LinkObjectsEObj>)
target_link_libraries(LinkObjectsEObj PRIVATE LinkObjectsEDep)
target_link_libraries(LinkObjectsEStatic PRIVATE LinkObjectsE)
target_link_libraries(LinkObjects PRIVATE LinkObjectsEStatic)
