include(FetchContent)

FetchContent_Declare(
  AddedProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
  OVERRIDE_FIND_PACKAGE
)

# The default generated config package files are expected to include these when present
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/AddedProjectExtra.cmake [[
message(STATUS "Uppercase extra file was read")
]]
)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/addedproject-extra.cmake [[
message(STATUS "Lowercase extra file was read")
]]
)

# This is expected to be re-routed to a FetchContent_MakeAvailable() call
find_package(AddedProject REQUIRED)

# Verify that find_package() version constraints are fully ignored by the
# default-generated config version file
find_package(AddedProject 1.2.3 EXACT REQUIRED)
