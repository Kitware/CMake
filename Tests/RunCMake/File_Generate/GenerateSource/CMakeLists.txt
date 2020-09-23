add_library(ObjLib OBJECT)

target_sources(ObjLib PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ObjLib.cpp")

# Ensure re-generation
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/ObjLib.cpp")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ObjLib.cpp" CONTENT "")
