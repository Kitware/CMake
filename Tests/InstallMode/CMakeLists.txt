cmake_minimum_required(VERSION 3.20.0)

project(superpro LANGUAGES NONE)

add_subdirectory(superpro)

include(Subproject.cmake)
add_subproject(static_lib   DIR subpro_a_static_lib)
add_subproject(shared_lib   DIR subpro_b_shared_lib)
add_subproject(nested_lib   DIR subpro_c_nested_lib NO_INSTALL)
add_subproject(executable   DIR subpro_d_executable
  DEPENDS
    static_lib
    shared_lib
    nested_lib
)

include(CTest)
if(BUILD_TESTING)
  enable_language(CXX)  # required by GNUInstallDirs
  include(GNUInstallDirs)

  macro(testme _name _path _symlink)
    add_test(
      NAME "${_name}"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMAND
        "${CMAKE_COMMAND}"
        "-DFILE_PATH=${CMAKE_INSTALL_PREFIX}/${_path}"
        "-DEXPECT_SYMLINK:BOOL=${_symlink}"
        "-DEXPECT_ABSOLUTE:BOOL=${ARGN}"
        "-P" "${CMAKE_SOURCE_DIR}/Test.cmake"
    )
  endmacro()

  set(_mode $ENV{CMAKE_INSTALL_MODE})
  if(NOT "${_mode}" OR "${_mode}" STREQUAL "COPY")
    set(expect_symlink NO)
  elseif("${_mode}" MATCHES "(REL_)?SYMLINK(_OR_COPY)?")
    set(expect_symlink YES)
    set(expect_absolute NO)
  elseif("${_mode}" MATCHES "ABS_SYMLINK(_OR_COPY)?")
    set(expect_symlink YES)
    set(expect_absolute YES)
  endif()

  # toplevel project should respect CMAKE_INSTALL_MODE

  testme(superproj_file_copy
    "file_copy.txt" NO)
  testme(superproj_file_copy_file
    "file_copy_file.txt" NO)
  testme(superproj_file_install
    "file_install.txt"
    ${expect_symlink}
    ${expect_absolute})
  testme(superproj_file_create_link_symbolic
    "file_create_link_symbolic.txt" YES YES)

  # subprojects should receive and respect CMAKE_INSTALL_MODE too

  testme(subpro_a_static_lib_header
    "${CMAKE_INSTALL_INCLUDEDIR}/static_lib.h"
    ${expect_symlink}
    ${expect_absolute}
  )
  testme(subpro_a_static_lib_libfile
    "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}the_static_lib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    ${expect_symlink}
    ${expect_absolute}
  )

  testme(subpro_b_shared_lib_header
    "${CMAKE_INSTALL_INCLUDEDIR}/shared_lib.h"
    ${expect_symlink}
    ${expect_absolute}
  )

  if(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG AND
      "${CMAKE_CXX_CREATE_SHARED_MODULE}" MATCHES "SONAME_FLAG")
    # due to semver, this is always a link
    testme(subpro_b_shared_lib_libfile
      "${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}the_shared_lib${CMAKE_SHARED_LIBRARY_SUFFIX}"
      YES
      ${expect_absolute}
    )
    # this is the actual shared lib, so should follow CMAKE_INSTALL_MODE rules
    testme(subpro_b_shared_lib_libfile_versuffix
      "${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}the_shared_lib${CMAKE_SHARED_LIBRARY_SUFFIX}.2.3.4"
      ${expect_symlink}
      ${expect_absolute}
    )
  endif()

  testme(subpro_d_executable_exefile
    "${CMAKE_INSTALL_BINDIR}/the_executable${CMAKE_EXECUTABLE_SUFFIX}"
    ${expect_symlink}
    ${expect_absolute}
  )

  # nested subprojects should receive and respect CMAKE_INSTALL_MODE too

  testme(subsubpro_c1_header
    "${CMAKE_INSTALL_INCLUDEDIR}/c1_lib.h"
    ${expect_symlink}
    ${expect_absolute}
  )
  testme(subsubpro_c1_libfile
    "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}the_c1_lib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    ${expect_symlink}
    ${expect_absolute}
  )

  testme(subsubpro_c2_header
    "${CMAKE_INSTALL_INCLUDEDIR}/c2_lib.h"
    ${expect_symlink}
    ${expect_absolute}
  )
  testme(subsubpro_c2_libfile
    "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}the_c2_lib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    ${expect_symlink}
    ${expect_absolute}
  )
endif()
