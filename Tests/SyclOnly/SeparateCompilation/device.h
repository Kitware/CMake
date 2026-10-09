#pragma once

#include <sycl/sycl.hpp>

// Upstream libsycl does not yet provide the SYCL_EXTERNAL convenience macro,
// but its compiler supports the underlying attribute.
#ifndef SYCL_EXTERNAL
#  if __has_cpp_attribute(clang::sycl_external)
#    define SYCL_EXTERNAL [[clang::sycl_external]]
#  else
#    error "The SYCL implementation does not support external device functions"
#  endif
#endif

SYCL_EXTERNAL int device_value(int value);
SYCL_EXTERNAL int device_offset();
int launch_kernel();
