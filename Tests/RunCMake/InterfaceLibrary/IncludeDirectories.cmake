cmake_policy(SET CMP0076 NEW)
include_directories(Inc1 Inc2)
add_library(iface INTERFACE)
target_sources(iface PRIVATE iface.c)
# Ensure the INCLUDE_DIRECTORIES property is populated.
# Since interface libraries do not actually compile anything, this should be ignored.
set_property(TARGET iface APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/Inc3 ${CMAKE_CURRENT_SOURCE_DIR}/Inc4)
