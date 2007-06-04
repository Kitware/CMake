project(Framework)
add_library(foo SHARED foo.cxx foo.h )
set_target_properties(foo PROPERTIES 
  FRAMEWORK TRUE
  FRAMEWORK_PUBLIC_HEADERS "foo.h;foo2.h"
  FRAMEWORK_VERSION ver2
  FRAMEWORK_RESOURCES "test.lua"
)
add_executable(bar bar.cxx)
target_link_libraries(bar foo)
 
