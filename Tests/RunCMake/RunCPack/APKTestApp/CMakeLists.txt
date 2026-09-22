cmake_minimum_required(VERSION 4.0)

project(CPackAPKGenerator
  LANGUAGES CXX
  VERSION "1.2.3"
)

# An Android application is a shared library loaded by the Java runtime.
add_library(app SHARED main.cpp)

if(CMAKE_ANDROID_ARCH_ABI)
  set(app_destination lib/${CMAKE_ANDROID_ARCH_ABI})
else()
  set(app_destination lib)
endif()

# On a host build, where the toolchain file is not in play, a shared library
# may well be a `RUNTIME` artifact instead.
install(TARGETS app
  LIBRARY DESTINATION ${app_destination}
  RUNTIME DESTINATION ${app_destination}
)
if(APK_TEST_DEFAULT_DIRECTORIES)
  # Neither the manifest nor the project sets the target API level, which
  # then defaults to the compile API level.
  file(READ AndroidManifest.xml manifest)
  string(REPLACE " android:targetSdkVersion=\"34\"" "" manifest "${manifest}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" "${manifest}")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml"
    DESTINATION .)
else()
  install(FILES AndroidManifest.xml DESTINATION .)
endif()
install(DIRECTORY res assets DESTINATION .)

# An Android archive with a manifest, resources, assets and native libraries
# for several ABIs, but no code.
set(aar_contents "${CMAKE_CURRENT_BINARY_DIR}/aar-contents")
file(COPY aar/ DESTINATION "${aar_contents}")
foreach(abi IN ITEMS arm64-v8a armeabi-v7a x86 x86_64)
  file(WRITE "${aar_contents}/jni/${abi}/libaar.so" "not a real library\n")
endforeach()
# Native libraries are stripped, unless they are to keep their debug
# symbols.  In a host build, which has no ELF library to strip, a stand-in
# for llvm-strip is given ELF look-alikes.  A stand-in for llvm-readelf
# finds one of them linked to the shared C++ runtime, which is packaged from
# a stand-in NDK for its ABI.
if(NOT CMAKE_ANDROID_ARCH_ABI)
  string(ASCII 127 elf_magic)
  file(WRITE "${aar_contents}/jni/x86_64/libstripped.so"
    "${elf_magic}ELF debug\nneeded libc++_shared.so\n")
  file(WRITE "${aar_contents}/jni/x86_64/libkept.so" "${elf_magic}ELF debug\n")
  set(CPACK_APK_STRIP_EXECUTABLE
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/strip.cmake")
  set(CPACK_READELF_EXECUTABLE
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/readelf.cmake")
  set(CPACK_APK_KEEP_DEBUG_SYMBOLS "libke?t*.so")
  set(CPACK_APK_NDK_ROOT "${CMAKE_CURRENT_BINARY_DIR}/ndk")
  set(ndk_libraries
    "${CPACK_APK_NDK_ROOT}/toolchains/llvm/prebuilt/host/sysroot/usr/lib")
  foreach(triple IN ITEMS aarch64-linux-android x86_64-linux-android)
    file(WRITE "${ndk_libraries}/${triple}/libc++_shared.so"
      "${elf_magic}ELF libc++\n")
  endforeach()
endif()
# Like those built reproducibly, the archives share their time stamps.
set(aar_mtime "2020-01-01 00:00:00Z")
set(aar "${CMAKE_CURRENT_BINARY_DIR}/testlib.aar")
add_custom_command(OUTPUT "${aar}"
  COMMAND ${CMAKE_COMMAND} -E tar cf "${aar}" --format=zip
    "--mtime=${aar_mtime}" AndroidManifest.xml res assets jni
  WORKING_DIRECTORY "${aar_contents}"
  VERBATIM
)

# A second archive, with an asset replacing that of the first, and
# components the first one or the application declares as well.
set(aar2_contents "${CMAKE_CURRENT_BINARY_DIR}/aar2-contents")
file(COPY aar2/ DESTINATION "${aar2_contents}")
file(WRITE "${aar2_contents}/assets/lib.txt" "testlib2 asset
")
set(aar2 "${CMAKE_CURRENT_BINARY_DIR}/testlib2.aar")
add_custom_command(OUTPUT "${aar2}"
  COMMAND ${CMAKE_COMMAND} -E tar cf "${aar2}" --format=zip
    "--mtime=${aar_mtime}" AndroidManifest.xml assets
  WORKING_DIRECTORY "${aar2_contents}"
  VERBATIM
)
add_custom_target(aar ALL DEPENDS "${aar}" "${aar2}")
install(FILES "${aar}" "${aar2}" DESTINATION .)

# Create APK package
set(CPACK_GENERATOR APK)
set(CPACK_PACKAGE_NAME APKGeneratorTest)
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_VENDOR "ACME Inc")
set(CPACK_PACKAGE_DESCRIPTION "An APK package for testing CMake's CPack APK generator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A test APK package")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.example.com")

# APK generator variables
# Like those of Gradle projects, the manifest does not name the package.
set(CPACK_APK_PACKAGE com.example.app)
set(CPACK_APK_JAVA_SOURCE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/java")
set(CPACK_APK_MANIFEST_PLACEHOLDERS "flavor=test-flavor")
set(CPACK_APK_VERSION_CODE 42)
if(NOT APK_TEST_DEFAULT_DIRECTORIES)
  set(CPACK_APK_TARGET_SDK_VERSION 35)
endif()
# A relative path is relative to the directory cpack runs in, not to the one
# the build tools run in.
if(NOT APK_TEST_DEFAULT_DIRECTORIES)
  file(COPY res-overlay DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
  set(CPACK_APK_RESOURCE_DIRECTORIES
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    res-overlay
  )
endif()
if(APK_TEST_MAVEN)
  include(maven.cmake)
endif()

include(CPack)
