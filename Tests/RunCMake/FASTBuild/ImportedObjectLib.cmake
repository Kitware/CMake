add_library(ExternalObjLib OBJECT IMPORTED)
set_target_properties(ExternalObjLib PROPERTIES
    IMPORTED_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/dummy.o"
)

add_executable(MyApp main.cpp)

target_link_libraries(MyApp PRIVATE $<TARGET_OBJECTS:ExternalObjLib>)
