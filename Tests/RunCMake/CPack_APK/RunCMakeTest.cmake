include(RunCPack) # Uses sample projects from `../RunCPack/*`

set(RunCPack_GENERATORS APK)

# Generate the debug keystore here rather than in the user's home.
set(ENV{ANDROID_USER_HOME} "${RunCMake_BINARY_DIR}/android-user-home")

# Packaging needs a real Android SDK and a JDK.  Building the native library
# for a device additionally needs an Android toolchain file.  The one of the
# NDK warns about the CMake versions it declares compatibility with.
if(CMake_TEST_CPACK_APK_TOOLCHAIN_FILE)
  list(APPEND RunCMake_TEST_OPTIONS
    "-DCMAKE_TOOLCHAIN_FILE=${CMake_TEST_CPACK_APK_TOOLCHAIN_FILE}"
    -Wno-deprecated)
endif()
if(CMake_TEST_CPACK_APK_SDK_ROOT)
  list(APPEND RunCMake_TEST_OPTIONS
    "-DCPACK_APK_SDK_ROOT=${CMake_TEST_CPACK_APK_SDK_ROOT}")
endif()

run_cpack(APKTestApp BUILD)

# The resource and asset directories installed to the packaging tree are
# used when none are set.
block()
  list(APPEND RunCMake_TEST_OPTIONS -DAPK_TEST_DEFAULT_DIRECTORIES=ON)
  run_cpack(APKDefaultDirectories SAMPLE APKTestApp BUILD)
endblock()

# Maven dependencies are resolved from a local repository.
block()
  list(APPEND RunCMake_TEST_OPTIONS -DAPK_TEST_MAVEN=ON)
  run_cpack(APKMaven SAMPLE APKTestApp BUILD)
endblock()
block()
  list(APPEND RunCMake_TEST_OPTIONS -DAPK_TEST_MAVEN=ON
    -DAPK_TEST_MAVEN_BAD_HASH=ON)
  run_cpack(APKMavenBadHash SAMPLE APKTestApp BUILD NO_VERIFY)
endblock()

# A superbuild builds the application once for each ABI and packages the
# native libraries of all of them into one APK.
if(CMake_TEST_CPACK_APK_TOOLCHAIN_FILE)
  block()
    list(REMOVE_ITEM RunCMake_TEST_OPTIONS
      "-DCMAKE_TOOLCHAIN_FILE=${CMake_TEST_CPACK_APK_TOOLCHAIN_FILE}")
    list(APPEND RunCMake_TEST_OPTIONS
      "-DAPK_TEST_TOOLCHAIN_FILE=${CMake_TEST_CPACK_APK_TOOLCHAIN_FILE}")
    run_cpack(APKSuperBuild BUILD)
  endblock()
endif()
