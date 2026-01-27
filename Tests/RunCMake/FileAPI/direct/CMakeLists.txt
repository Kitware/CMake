project(Direct)

add_executable(link_transitive_direct_exe ../empty.c)
add_library(transitive_direct_lib STATIC ../empty.c)
add_library(inject_direct_lib INTERFACE)
add_library(inject_direct_lib_impl OBJECT other.c)

set(head_targets
  EXECUTABLE
  SHARED_LIBRARY
  MODULE_LIBRARY
)
set_property(TARGET inject_direct_lib PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT
  # Skip intermediate targets, only apply to the head target
  "$<$<IN_LIST:$<TARGET_PROPERTY:TYPE>,${head_targets}>:inject_direct_lib_impl>"
)

target_link_libraries(link_transitive_direct_exe
  # Link interfaces are not supported for executables that do not export
  # symbols, so even though this specifies PUBLIC here, the direct dependencies
  # should show up as private.
  PUBLIC
    transitive_direct_lib
)

# This is not an executable, so the direct dependency types should follow the
# keywords specified here.
target_link_libraries(transitive_direct_lib
  PUBLIC
    inject_direct_lib
  INTERFACE
    -lm  # Doesn't need to exist, just verifying fragments in directDependencies
)

cmake_policy(SET CMP0131 NEW)
add_library(usage_lib STATIC ../empty.c)
add_executable(link_usage_exe ../empty.c)
add_executable(compile_usage_exe ../empty.c)
target_link_libraries(link_usage_exe PRIVATE $<LINK_ONLY:usage_lib>)
target_link_libraries(compile_usage_exe PRIVATE $<COMPILE_ONLY:usage_lib>)
