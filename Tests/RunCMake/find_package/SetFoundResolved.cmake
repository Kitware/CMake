# Create ./symlink pointing back here.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  PackageRoot "${CMAKE_CURRENT_SOURCE_DIR}/symlink")

# Make find_package search through the symlink.
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/symlink")

# Test preservation of symlinks.
find_package(Resolved)
message(WARNING "${Resolved_DIR}")

# Test resolving symlinks.
set(CMAKE_FIND_PACKAGE_RESOLVE_SYMLINKS ON)
find_package(Resolved)
message(WARNING "${Resolved_DIR}")

file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/symlink")
