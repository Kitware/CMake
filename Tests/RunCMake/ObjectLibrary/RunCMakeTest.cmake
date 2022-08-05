include(RunCMake)

run_cmake(BadSourceExpression1)
run_cmake(BadSourceExpression2)
run_cmake(BadSourceExpression3)
run_cmake(BadObjSource1)
run_cmake(BadObjSource2)
if(RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  run_cmake(ImportMultiArch)
  run_cmake(InstallNotSupported)

  set(osx_archs $ENV{CMAKE_OSX_ARCHITECTURES})
  list(GET osx_archs 0 osx_arch)
  run_cmake_with_options(TargetOverrideSingleArch -Dosx_arch=${osx_arch})
else()
  run_cmake(Import)
  run_cmake(Install)
  run_cmake(InstallLinkedObj1)
  run_cmake(InstallLinkedObj2)

  if(RunCMake_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_GREATER_EQUAL 13)
    run_cmake(TargetOverrideMultiArch)
  endif()
endif()

run_cmake(Export)

function (run_object_lib_build name)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${name})
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build .)
endfunction ()

function (run_object_lib_build2 name)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${name})
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build .)
endfunction ()

if(NOT (RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]"))
  run_object_lib_build(CheckTargetObjects)
endif()

run_object_lib_build(LinkObjLHSShared)
run_object_lib_build(LinkObjLHSStatic)
run_object_lib_build(LinkObjRHSShared)
run_object_lib_build(LinkObjRHSStatic)
run_object_lib_build2(LinkObjRHSObject)
run_object_lib_build(LinkObjRHSShared2)
run_object_lib_build(LinkObjRHSStatic2)
run_object_lib_build2(LinkObjRHSObject2)
run_object_lib_build(TransitiveDependencies)

run_cmake(MissingSource)
run_cmake(ObjWithObj)
run_cmake(OwnSources)
run_cmake(PostBuild)
run_cmake(PreBuild)
run_cmake(PreLink)


function(run_Dependencies)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Dependencies-build)
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  if(RunCMake_GENERATOR STREQUAL "Borland Makefiles" OR
      RunCMake_GENERATOR STREQUAL "Watcom WMake")
    set(fs_delay 3)
  else()
    set(fs_delay 1.125)
  endif()

  run_cmake_command(Dependencies-build ${CMAKE_COMMAND} -E copy ${RunCMake_SOURCE_DIR}/depends_obj1.c ${RunCMake_TEST_BINARY_DIR}/depends_obj.c)
  run_cmake(Dependencies)
  run_cmake_command(Dependencies-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(Dependencies-build ${CMAKE_COMMAND} -E sleep ${fs_delay})
  run_cmake_command(Dependencies-build ${CMAKE_COMMAND} -E copy ${RunCMake_SOURCE_DIR}/depends_obj0.c ${RunCMake_TEST_BINARY_DIR}/depends_obj.c)
  run_cmake_command(Dependencies-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(Dependencies-build ${CMAKE_CTEST_COMMAND} -C Debug -V)
endfunction()

run_Dependencies()
