cpack-apk-generator
-------------------

* The :cpack_gen:`CPack APK Generator` was added to package Android
  applications as APK files.  It drives the Android SDK build tools
  directly and does not require Gradle.  Maven dependencies, such as the
  AndroidX libraries, can be downloaded with
  :variable:`CPACK_APK_MAVEN_DEPENDENCIES`.
