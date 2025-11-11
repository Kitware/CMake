set(CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO
  cow:Farm/l/cps
  pig:Farm/laextra/cps
)

add_library(Cow INTERFACE)
add_library(Pig INTERFACE)

install(TARGETS Cow EXPORT cow)
install(TARGETS Pig EXPORT pig)
install(EXPORT cow FILE farm-targets.cmake DESTINATION .)
install(EXPORT pig FILE farm-targets-extra.cmake DESTINATION .)
