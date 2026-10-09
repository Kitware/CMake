add_library(fileset_experimental INTERFACE)
target_sources(fileset_experimental INTERFACE
  FILE_SET sycl_sources TYPE SYCL)
