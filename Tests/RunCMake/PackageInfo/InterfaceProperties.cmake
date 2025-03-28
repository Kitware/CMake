add_library(foo INTERFACE)

target_compile_features(foo INTERFACE cxx_std_23)
target_compile_options(foo INTERFACE -ffast-math)
target_compile_definitions(foo INTERFACE -DFOO -DBAR=BAR)
target_include_directories(
  foo INTERFACE
  $<INSTALL_INTERFACE:include/foo>
  )
target_link_directories(foo INTERFACE /opt/foo/lib)
target_link_options(foo INTERFACE --needed)
target_link_libraries(foo INTERFACE /usr/lib/libm.so)

install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo)
