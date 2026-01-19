enable_language(C)

set(CMAKE_VERIFY_INTERFACE_HEADER_SETS TRUE)
set(CMAKE_VERIFY_PRIVATE_HEADER_SETS TRUE)

include(FetchContent)
FetchContent_Declare(verify_subproj
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/VerifyHeaderSet
)
message(STATUS "Before subproject, interface var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")
message(STATUS "Before subproject, private var = '${CMAKE_VERIFY_PRIVATE_HEADER_SETS}'")
FetchContent_MakeAvailable(verify_subproj)

# Provide a way to verify the variable was reset back to its original value
message(STATUS "After subproject, interface var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")
message(STATUS "After subproject, private var = '${CMAKE_VERIFY_PRIVATE_HEADER_SETS}'")

get_property(verify_interface TARGET Blah PROPERTY VERIFY_INTERFACE_HEADER_SETS)
get_property(verify_private TARGET Blah PROPERTY VERIFY_PRIVATE_HEADER_SETS)
message(STATUS "Subproject target property VERIFY_INTERFACE_HEADER_SETS='${verify_interface}'")
message(STATUS "Subproject target property VERIFY_PRIVATE_HEADER_SETS='${verify_private}'")
