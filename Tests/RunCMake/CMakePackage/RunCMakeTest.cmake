include(RunCMake)

if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(maybe_CMAKE_BUILD_TYPE -DCMAKE_BUILD_TYPE=Release)
endif()

function(apple_export platform system_name archs sysroot)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/apple-export-${platform}-build)
  string(REPLACE ";" "\\;" archs "${archs}")
  if(select_archs)
    string(REPLACE ";" "\\\\;" maybe_IOS_SIMULATOR_SELECT_ARCHS "-DIOS_SIMULATOR_SELECT_ARCHS=${select_archs}")
  endif()
  run_cmake_with_options(apple-export-${platform}
    "-DCMAKE_SYSTEM_NAME=${system_name}"
    "-DCMAKE_OSX_ARCHITECTURES=${archs}"
    "-DCMAKE_OSX_SYSROOT=${sysroot}"
    "-DCMAKE_INSTALL_PREFIX=${apple_install}"
    ${maybe_CMAKE_BUILD_TYPE}
    ${maybe_IOS_SIMULATOR_SELECT_ARCHS}
    )
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(apple-export-${platform}-build ${CMAKE_COMMAND} --build . --config Release)
  run_cmake_command(apple-export-${platform}-install ${CMAKE_COMMAND} --install . --config Release)
  file(APPEND "${apple_install}/lib/${platform}/cmake/mylib/mylib-targets.cmake" "\n"
    "message(STATUS \"loaded: '\${CMAKE_CURRENT_LIST_FILE}'\")\n"
    )
endfunction()

function(apple_import platform system_name archs sysroot)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/apple-import-${platform}-build)
  string(REPLACE ";" "\\;" archs "${archs}")
  run_cmake_with_options(apple-import-${platform}
    "-DCMAKE_SYSTEM_NAME=${system_name}"
    "-DCMAKE_OSX_ARCHITECTURES=${archs}"
    "-DCMAKE_OSX_SYSROOT=${sysroot}"
    "-DCMAKE_PREFIX_PATH=${apple_install}"
    ${maybe_CMAKE_BUILD_TYPE}
    )
  set(RunCMake_TEST_NO_CLEAN 1)
  if(apple_import_no_build)
    return()
  endif()
  run_cmake_command(apple-import-${platform}-build ${CMAKE_COMMAND} --build . --config Release)
endfunction()

if(APPLE)
  run_cmake(ApplePlatformMissingDest)
endif()

if(APPLE AND CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
  if(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 12)
    set(macos_archs "x86_64;arm64")
    set(tvos_sim_archs "x86_64;arm64")
    set(watch_sim_archs "x86_64")
    set(select_archs "arm64;x86_64")
  elseif(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 10)
    set(macos_archs "x86_64")
    set(tvos_sim_archs "x86_64")
    set(watch_sim_archs "i386")
    set(select_archs "")
  else()
    set(macos_archs "i386;x86_64")
    set(tvos_sim_archs "x86_64")
    set(watch_sim_archs "i386")
    set(select_archs "")
  endif()

  if(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 9)
    set(watch_archs "armv7k;arm64_32")
  else()
    set(watch_archs "armv7k")
  endif()

  if(CMake_TEST_XCODE_VERSION VERSION_GREATER_EQUAL 15.2)
    set(enable_visionos 1)
  endif()

  string(REPLACE ";" "\\;" macos_archs_for_cmd "${macos_archs}")
  run_cmake_with_options(ApplePlatformGenSubdir
    "-DCMAKE_OSX_ARCHITECTURES=${macos_archs_for_cmd}"
    ${maybe_CMAKE_BUILD_TYPE}
  )
  unset(macos_archs_for_cmd)

  # Place all export/import steps in a single install prefix.
  set(apple_install ${RunCMake_BINARY_DIR}/apple-install)
  file(REMOVE_RECURSE "${apple_install}")

  apple_export(macos Darwin "${macos_archs}" macosx)
  apple_export(ios iOS "arm64" iphoneos)
  apple_export(tvos tvOS "arm64" appletvos)
  if(enable_visionos)
    apple_export(visionos visionOS "arm64" xros)
  endif()
  apple_export(watchos watchOS "${watch_archs}" watchos)
  apple_export(ios-simulator iOS "${macos_archs}" iphonesimulator)
  if(select_archs)
    foreach(arch IN LISTS macos_archs)
      apple_export(ios-simulator-${arch} iOS "${arch}" iphonesimulator)
    endforeach()
  endif()
  apple_export(tvos-simulator tvOS "${tvos_sim_archs}" appletvsimulator)
  if(enable_visionos)
    apple_export(visionos-simulator visionOS "${macos_archs}" xrsimulator)
  endif()
  apple_export(watchos-simulator watchOS "${watch_sim_archs}" watchsimulator)
  apple_export(unsupported-capture Darwin "${macos_archs}" macosx)
  apple_export(unsupported-fatal Darwin "${macos_archs}" macosx)

  apple_import(macos Darwin "${macos_archs}" macosx)
  apple_import(ios iOS "arm64" iphoneos)
  apple_import(tvos tvOS "arm64" appletvos)
  if(enable_visionos)
    apple_import(visionos visionOS "arm64" xros)
  endif()
  apple_import(watchos watchOS "${watch_archs}" watchos)
  apple_import(ios-simulator iOS "${macos_archs}" iphonesimulator)
  if(RunCMake_GENERATOR STREQUAL "Xcode")
    apple_import(ios-simulator-xcode iOS "$(ARCHS_STANDARD)" iphonesimulator)
  endif()
  if(select_archs)
    foreach(arch IN LISTS macos_archs)
      apple_import(ios-simulator-${arch} iOS "${arch}" iphonesimulator)
    endforeach()
  endif()
  apple_import(tvos-simulator tvOS "${tvos_sim_archs}" appletvsimulator)
  if(enable_visionos)
    apple_import(visionos-simulator visionOS "${macos_archs}" xrsimulator)
  endif()
  apple_import(watchos-simulator watchOS "${watch_sim_archs}" watchsimulator)
  set(apple_import_no_build 1)
  apple_import(unsupported-capture Darwin "${macos_archs}" macosx)
  apple_import(unsupported-fatal-platform Darwin "${macos_archs}" macosx)
  apple_import(unsupported-fatal-architecture Darwin "${macos_archs}" macosx)
  unset(apple_import_no_build)
endif()
