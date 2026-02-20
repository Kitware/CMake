add_library(foo INTERFACE)

target_compile_features(foo INTERFACE c_std_17 cxx_std_23)
target_compile_options(foo INTERFACE -ffast-math)
target_compile_definitions(foo INTERFACE -DFOO -DBAR=BAR)
target_include_directories(
  foo INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/foo>
  )
target_link_directories(foo INTERFACE /opt/foo/lib)
target_link_options(foo INTERFACE --needed)
target_link_libraries(foo INTERFACE /usr/lib/libm.so)
set_property(TARGET foo PROPERTY SPDX_LICENSE "BSD-3-Clause")

install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO foo EXPORT foo)
