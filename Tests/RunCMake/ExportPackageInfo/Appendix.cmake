add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS mammal EXPORT mammal DESTINATION .)
install(TARGETS canine EXPORT canine DESTINATION .)

export(
  PACKAGE_INFO foo
  EXPORT mammal
  VERSION 1.0
  DEFAULT_LICENSE "LGPL-3.0-or-later")

export(
  PACKAGE_INFO foo
  EXPORT canine
  APPENDIX dog
  DEFAULT_LICENSE "GPL-3.0-or-later")
