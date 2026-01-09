set(regular_root "${CMAKE_CURRENT_SOURCE_DIR}/FindRootPathAndPrefixPathAreEqual")
set(regular_prefix "${regular_root}/lib/cmake")

# 'emptydir' must be a real dir on the file system, otherwise CMake
# won't canonicalize the path when getting resolving the real path.
set(dotted_root "${regular_root}/emptydir/..")
set(dotted_prefix "${dotted_root}/lib/cmake")

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE "ONLY")

# Case with no '..' in the paths
set(CMAKE_FIND_ROOT_PATH "${regular_root}")
set(CMAKE_PREFIX_PATH "${regular_prefix}")
message(STATUS "Looking for Foo without '..' in the paths")
find_package(Foo
             REQUIRED
             CONFIG
             NO_CMAKE_ENVIRONMENT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH
             # Important because CMAKE_SYSTEM_PREFIX_PATH might contain "/" as a prefix
             # And when "/" is rerooted onto the root above, the package is found even if
             # CMAKE_PREFIX_PATH is empty. We want to ensure that we hit
             # the CMAKE_FIND_ROOT_PATH == CMAKE_PREFIX_PATH code path.
             NO_CMAKE_SYSTEM_PATH
             )
if(Foo_FOUND)
    message(STATUS "Foo found for case without '..' in the paths")
endif()

# Unset the cache variable to find the package again.
unset(Foo_DIR CACHE)

# Case with '..' in the paths
set(CMAKE_FIND_ROOT_PATH "${dotted_root}")
set(CMAKE_PREFIX_PATH "${dotted_prefix}")
message(STATUS "Looking for Foo with '..' in the paths")
find_package(Foo
             REQUIRED
             CONFIG
             NO_CMAKE_ENVIRONMENT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH
             # Important because CMAKE_SYSTEM_PREFIX_PATH might contain "/" as a prefix
             # And when "/" is rerooted onto the root above, the package is found even if
             # CMAKE_PREFIX_PATH is empty. We want to ensure that we hit
             # the CMAKE_FIND_ROOT_PATH == CMAKE_PREFIX_PATH code path.
             NO_CMAKE_SYSTEM_PATH
             )
if(Foo_FOUND)
    message(STATUS "Foo found for case with '..' in the paths")
endif()
