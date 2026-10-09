set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(CXX)
enable_language(SYCL)

add_library(fileset_export INTERFACE)
target_sources(fileset_export INTERFACE
  FILE_SET export_sources TYPE SYCL FILES FileSetInherited.cxx)
target_sources(fileset_export INTERFACE
  FILE_SET SYCL FILES FileSetDefaultInherited.cxx)

add_library(fileset_default_override_export INTERFACE)
target_sources(fileset_default_override_export INTERFACE
  FILE_SET SYCL TYPE SOURCES FILES FileSetDefaultCxx.cxx)
set_property(FILE_SET SYCL TARGET fileset_default_override_export
  PROPERTY LANGUAGE CXX)

add_library(fileset_headers_export INTERFACE)
target_sources(fileset_headers_export INTERFACE
  FILE_SET SYCL_HEADERS TYPE HEADERS FILES FileSetViral.h)

export(TARGETS fileset_export fileset_default_override_export
  fileset_headers_export
  FILE export.cmake NAMESPACE export::)

install(TARGETS fileset_export EXPORT fileset_install
  FILE_SET export_sources DESTINATION src/export
  FILE_SET SYCL DESTINATION src/default)
install(TARGETS fileset_default_override_export EXPORT fileset_install
  FILE_SET SYCL DESTINATION src/default-cxx)
install(TARGETS fileset_headers_export EXPORT fileset_install
  FILE_SET SYCL_HEADERS DESTINATION include)
install(EXPORT fileset_install FILE install-export.cmake
  NAMESPACE install:: DESTINATION lib/cmake)
