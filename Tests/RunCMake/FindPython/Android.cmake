enable_language(C)

set(components Development.Embed Development.Module Development.SABIModule)
find_package(Python REQUIRED COMPONENTS ${components})

foreach(component ${components})
  set(found_var Python_${component}_FOUND)
  if(NOT ${found_var})
    message(FATAL_ERROR "${found_var} is not set")
  endif()
endforeach()

set(android_root "${CMAKE_SOURCE_DIR}/android_root")
if(NOT Python_INCLUDE_DIRS STREQUAL "${android_root}/include/python3.13")
  message(FATAL_ERROR "Python_INCLUDE_DIRS=${Python_INCLUDE_DIRS}")
endif()
if(NOT Python_LIBRARIES STREQUAL "${android_root}/lib/libpython3.13.so")
  message(FATAL_ERROR "Python_LIBRARIES=${Python_LIBRARIES}")
endif()
if(NOT Python_SABI_LIBRARIES STREQUAL "${android_root}/lib/libpython3.so")
  message(FATAL_ERROR "Python_SABI_LIBRARIES=${Python_SABI_LIBRARIES}")
endif()

foreach(target Python::Python Python::Module Python::SABIModule)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "Target ${target} does not exist")
  endif()

  # The Module and SABIModule targets will be SHARED_LIBRARY if Python modules should
  # link against libpython (as on Android), and INTERFACE_LIBRARY if they should not (as
  # on Linux).
  get_target_property(target_type ${target} TYPE)
  if(NOT target_type STREQUAL SHARED_LIBRARY)
    message(FATAL_ERROR "${target} TYPE=${target_type}")
  endif()

  get_target_property(target_location ${target} LOCATION)
  if(target STREQUAL "Python::SABIModule")
    set(expected "${android_root}/lib/libpython3.so")
  else()
    set(expected "${android_root}/lib/libpython3.13.so")
  endif()
  if(NOT target_location STREQUAL expected)
    message(FATAL_ERROR "${target} LOCATION=${target_location}")
  endif()

  get_target_property(target_include ${target} INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT target_include STREQUAL "${android_root}/include/python3.13")
    message(FATAL_ERROR "${target} INTERFACE_INCLUDE_DIRECTORIES=${target_include}")
  endif()
endforeach()
