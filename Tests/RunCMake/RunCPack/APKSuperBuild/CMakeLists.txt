cmake_minimum_required(VERSION 4.0)
cmake_policy(SET CMP0225 NEW)

# Build the application once for each ABI, from the same sources, and
# package all of them into one APK.
project(APKSuperBuild
  LANGUAGES NONE
  VERSION "1.2.3"
)

include(ExternalProject)

set(abis arm64-v8a x86_64)
foreach(abi IN LISTS abis)
  set(binary_dir "${CMAKE_CURRENT_BINARY_DIR}/app-${abi}")
  ExternalProject_Add(app-${abi}
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../APKTestApp"
    BINARY_DIR "${binary_dir}"
    CMAKE_ARGS
      "-DCMAKE_TOOLCHAIN_FILE=${APK_TEST_TOOLCHAIN_FILE}"
      -DANDROID_ABI=${abi}
      -DCMAKE_BUILD_TYPE=$<CONFIG>
      -Wno-deprecated
    BUILD_ALWAYS TRUE
    INSTALL_COMMAND ""
  )
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS
    "${binary_dir};CPackAPKGenerator;ALL;/")
endforeach()

# The NDK of the toolchain file provides llvm-strip, llvm-readelf and the
# shared C++ runtime.
cmake_path(GET APK_TEST_TOOLCHAIN_FILE PARENT_PATH CPACK_APK_NDK_ROOT)
cmake_path(GET CPACK_APK_NDK_ROOT PARENT_PATH CPACK_APK_NDK_ROOT)
cmake_path(GET CPACK_APK_NDK_ROOT PARENT_PATH CPACK_APK_NDK_ROOT)

set(CPACK_GENERATOR APK)
set(CPACK_PACKAGE_NAME APKGeneratorTest)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}")
set(CPACK_APK_PACKAGE com.example.app)
set(CPACK_APK_JAVA_SOURCE_DIRECTORIES
  "${CMAKE_CURRENT_SOURCE_DIR}/../APKTestApp/java")
set(CPACK_APK_MANIFEST_PLACEHOLDERS "flavor=test-flavor")

include(CPack)
