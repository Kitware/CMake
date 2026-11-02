project(DefaultLicense SPDX_LICENSE GPL-3.0-only LANGUAGES NONE)

add_library(explicit_license INTERFACE)
set_property(TARGET explicit_license PROPERTY SPDX_LICENSE Apache-2.0)
add_library(default_license INTERFACE)
add_library(empty_license INTERFACE)
set_property(TARGET empty_license PROPERTY SPDX_LICENSE "")

install(TARGETS explicit_license default_license empty_license
  EXPORT licenses DESTINATION .)
