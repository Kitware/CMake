add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS mammal EXPORT mammal DESTINATION .)
install(TARGETS canine EXPORT canine DESTINATION .)

export(
  EXPORT mammal
  PACKAGE_INFO foo
  VERSION 1.0
  DEFAULT_LICENSE "LGPL-3.0-or-later")

export(
  EXPORT canine
  PACKAGE_INFO foo
  APPENDIX dog
  DEFAULT_LICENSE "GPL-3.0-or-later")
