#pragma once

#include <iostream>

#include <sycl/sycl.hpp>

inline bool sycl_test_device_available()
{
  if (sycl::device::get_devices().empty()) {
    std::cout << "No SYCL device available\n";
    return false;
  }
  return true;
}
