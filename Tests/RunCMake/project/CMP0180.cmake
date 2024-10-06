add_subdirectory(subdir1)

# Simulate a situation that FetchContent_MakeAvailable() used to be able to
# create, but that should no longer be possible. If depname_SOURCE_DIR and
# depname_BINARY_DIR variables are defined as non-cache variables before the
# project(depname) call, those non-cache variables used to prevent project()
# from setting those variables itself due to CMP0126 (if set to NEW). This only
# showed up if the project(depname) call was not in the dependency's top level
# CMakeLists.txt file, but rather in a subdirectory (googletest is one example
# that used to do this). Since CMake 3.30.3, the dependency's project() call
# should set non-cache variables that will make the variable values visible
# and avoid any masking from variables set before the project() call. We want
# to verify this 3.30.3+ behavior here and in subdir2.
set(sub2proj_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(sub2proj_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(subdir2)
