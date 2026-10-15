# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# LLVM Flang requires Darwin linker flags to be passed through its linker
# wrapper rather than directly to the compiler driver.
if(CMAKE_Fortran_COMPILER_VERSION VERSION_LESS 20)
  # Flang < 20 has no -Xlinker.  The -Wl, form needs the install name joined
  # to its flag, without the space Darwin.cmake leaves after <SONAME_FLAG>.
  set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-Wl,-compatibility_version,")
  set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-Wl,-current_version,")
  set(CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS "-Wl,-bundle")
  set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Wl,-install_name,")
  foreach(rule IN ITEMS CREATE_SHARED_LIBRARY CREATE_MACOSX_FRAMEWORK)
    set(CMAKE_Fortran_${rule}
      "<CMAKE_Fortran_COMPILER> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG><TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
  endforeach()
else()
  set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-Xlinker -compatibility_version -Xlinker ")
  set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-Xlinker -current_version -Xlinker ")
  set(CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS "-Xlinker -bundle")
  set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Xlinker -install_name -Xlinker ")
endif()
