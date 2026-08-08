
# With CMP0220 NEW, CXX and its implicit RC language propagate up to this scope.
cmake_policy(SET CMP0220 NEW)

add_subdirectory(cxx)

add_executable(main rc-main.cxx resource.rc)
