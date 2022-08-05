enable_language(C)

set(CMAKE_VERIFY_INTERFACE_HEADER_SETS TRUE)

include(FetchContent)
FetchContent_Declare(verify_subproj
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/VerifyHeaderSet
)
message(STATUS "Before subproject, var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")
FetchContent_MakeAvailable(verify_subproj)

# Provide a way to verify the variable was reset back to its original value
message(STATUS "After subproject, var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")

get_property(verify TARGET Blah PROPERTY VERIFY_INTERFACE_HEADER_SETS)
message(STATUS "Subproject target property VERIFY_INTERFACE_HEADER_SETS='${verify}'")
