enable_language(C)

add_library(mainlib STATIC foo.c)
target_compile_definitions(mainlib INTERFACE
  $<BUILD_LOCAL_INTERFACE:BUILD_LOCAL_INTERFACE>
  $<BUILD_INTERFACE:BUILD_INTERFACE>
  $<INSTALL_INTERFACE:INSTALL_INTERFACE>
  )
add_library(locallib STATIC locallib.c)
target_link_libraries(locallib PRIVATE mainlib)

install(TARGETS mainlib EXPORT export)
install(EXPORT export DESTINATION lib/cmake/install FILE install-config.cmake NAMESPACE install::)
export(EXPORT export FILE build-config.cmake NAMESPACE build::)
