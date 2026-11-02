project(metadata VERSION 1.2.3 SPDX_LICENSE GPL-3.0-only
  DESCRIPTION "Project description" HOMEPAGE_URL https://project.example
  LANGUAGES NONE)

add_library(explicit_license INTERFACE)
set_property(TARGET explicit_license PROPERTY SPDX_LICENSE Apache-2.0)
add_library(no_license INTERFACE)

install(TARGETS explicit_license no_license EXPORT metadata DESTINATION .)
