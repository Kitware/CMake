set(root "${CMAKE_CURRENT_SOURCE_DIR}/FindRootPathAndPrefixPathAreEqual")
set(CMAKE_FIND_ROOT_PATH "${root}")
set(CMAKE_PREFIX_PATH "${root}")
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE "ONLY")

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
