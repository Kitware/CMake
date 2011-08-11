
if("${CMAKE_CXX_COMPILER_ID}" MATCHES GNU
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES Intel
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES Clang
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES XL
  OR "${CMAKE_CXX_COMPILER_ID}" MATCHES SunPro)

  find_package(PNG)

  # the test program links against the png lib, so test first whether it exists
  if(PNG_FOUND)

    get_target_property(cmakeExecutable cmake LOCATION)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in ${CMAKE_CURRENT_BINARY_DIR}/ConfMakefile @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_BINARY_DIR}/main.cpp COPYONLY)

    add_test(FindPackageModeMakefileTest ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_BINARY_DIR}/ConfMakefile -C ${CMAKE_CURRENT_BINARY_DIR} )

  endif()

endif()
