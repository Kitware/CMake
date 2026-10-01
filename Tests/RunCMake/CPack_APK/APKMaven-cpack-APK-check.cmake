include("${CMAKE_CURRENT_LIST_DIR}/APKTestApp-cpack-APK-check.cmake")

# The permission of the Android archive downloaded from the repository is
# merged into the manifest.
if(DEFINED manifest)
  string(FIND "${manifest}" "\"com.example.maven.PERMISSION\"" position)
  if(position EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED
      "The manifest lacks the permission of the Maven dependency\n")
  endif()
endif()

# Only the archives of the selected versions and variants are downloaded.
set(cache "${RunCMake_TEST_BINARY_DIR}/_CPack_Packages/maven")
file(GLOB_RECURSE archives RELATIVE "${cache}" "${cache}/*.aar" "${cache}/*.jar")
list(SORT archives)
set(expected_archives
  com/example/maven/app-dep/1.0/app-dep-1.0.aar
  com/example/maven/bom-managed/1.0/bom-managed-1.0.jar
  com/example/maven/constrained/2.0/constrained-2.0.jar
  com/example/maven/evicted/1.1/evicted-1.1.jar
  com/example/maven/evicting/1.0/evicting-1.0.jar
  com/example/maven/excluding/1.0/excluding-1.0.jar
  com/example/maven/kmp-android/1.0/kmp-android-1.0.jar
  com/example/maven/lowered/1.0/lowered-1.0.jar
  com/example/maven/pom-dep/2.0/pom-dep-2.0.jar
  com/example/maven/pom-managed/1.0/pom-managed-1.0.jar
  )
if(NOT archives STREQUAL expected_archives)
  string(APPEND RunCMake_TEST_FAILED
    "The Maven cache holds the archives\n  ${archives}\n"
    "instead of\n  ${expected_archives}\n")
endif()
