# Without the resource directories set, the staged ones are used, which the
# overlay is not part of.
if(NOT DEFINED expected_label)
  set(expected_label "APK Generator Test Overlay")
endif()

set(apk "${RunCMake_TEST_BINARY_DIR}/APKGeneratorTest-1.2.3.apk")
if(NOT EXISTS "${apk}")
  set(RunCMake_TEST_FAILED "APK package not generated")
  return()
endif()

# The APK is a zip; check that the pieces the generator is responsible for
# assembling are all in it.
set(contents "${RunCMake_TEST_BINARY_DIR}/apk-contents")
file(ARCHIVE_EXTRACT INPUT "${apk}" DESTINATION "${contents}")
foreach(entry IN ITEMS AndroidManifest.xml resources.arsc classes.dex
    assets/lib.txt assets/app.txt)
  if(NOT EXISTS "${contents}/${entry}")
    string(APPEND RunCMake_TEST_FAILED "APK does not contain ${entry}\n")
  endif()
endforeach()
file(GLOB_RECURSE signature "${contents}/META-INF/*.RSA")
if(NOT signature)
  string(APPEND RunCMake_TEST_FAILED "APK is not signed\n")
endif()

# The asset of the later archive replaces that of the earlier one, even
# though their time stamps are the same.
if(EXISTS "${contents}/assets/lib.txt")
  file(READ "${contents}/assets/lib.txt" asset)
  if(NOT asset STREQUAL "testlib2 asset
")
    string(APPEND RunCMake_TEST_FAILED
      "assets/lib.txt holds '${asset}' instead of the second archive's
")
  endif()
endif()

# The native libraries of the Android archive are only packaged for the ABI
# of the application, or for all of them when the application's own library
# is not in an ABI directory, as in a host build.
file(GLOB aar_libraries RELATIVE "${contents}/lib" "${contents}/lib/*/libaar.so")
list(TRANSFORM aar_libraries REPLACE "/libaar.so$" "")
file(GLOB app_libraries LIST_DIRECTORIES FALSE "${contents}/lib/*")
if(app_libraries)
  set(host_build 1)
  set(expected_abis arm64-v8a armeabi-v7a x86 x86_64)
else()
  file(GLOB_RECURSE app_libraries RELATIVE "${contents}/lib" "${contents}/lib/*")
  list(FILTER app_libraries EXCLUDE REGEX "/libaar\\.so$")
  list(TRANSFORM app_libraries REPLACE "/.*" "")
  list(REMOVE_DUPLICATES app_libraries)
  set(expected_abis "${app_libraries}")
endif()
if(NOT expected_abis)
  string(APPEND RunCMake_TEST_FAILED "APK does not contain any native library\n")
endif()
list(SORT aar_libraries)
list(SORT expected_abis)
if(NOT aar_libraries STREQUAL expected_abis)
  string(APPEND RunCMake_TEST_FAILED
    "APK holds the archive's native library for '${aar_libraries}', "
    "expected '${expected_abis}'\n")
endif()

# The ELF libraries are stripped, except those to keep the debug symbols
# of; the others are packaged as they are.  The host build gives the
# stand-in for llvm-strip ELF look-alikes.
if(host_build)
  string(ASCII 127 elf_magic)
  foreach(check IN ITEMS
      "libstripped.so|stripped --strip-unneeded\n"
      "libkept.so|${elf_magic}ELF debug\n"
      "libaar.so|not a real library\n")
    string(REPLACE "|" ";" check "${check}")
    list(GET check 0 library)
    list(GET check 1 expected)
    file(READ "${contents}/lib/x86_64/${library}" actual)
    if(NOT actual STREQUAL expected)
      string(APPEND RunCMake_TEST_FAILED
        "lib/x86_64/${library} holds '${actual}' instead of '${expected}'\n")
    endif()
  endforeach()

  # The shared C++ runtime of the NDK is packaged, stripped, for the ABI
  # whose libraries are linked to it, and only for that one.
  file(GLOB cxx_shared RELATIVE "${contents}/lib"
    "${contents}/lib/*/libc++_shared.so")
  if(NOT cxx_shared STREQUAL "x86_64/libc++_shared.so")
    string(APPEND RunCMake_TEST_FAILED
      "APK holds libc++_shared.so as '${cxx_shared}', expected "
      "'x86_64/libc++_shared.so'\n")
  else()
    file(READ "${contents}/lib/x86_64/libc++_shared.so" actual)
    if(NOT actual STREQUAL "stripped --strip-unneeded\n")
      string(APPEND RunCMake_TEST_FAILED
        "lib/x86_64/libc++_shared.so holds '${actual}' instead of the "
        "stripped runtime of the NDK\n")
    endif()
  endif()
endif()

# The R class of the archive's package is generated.
file(STRINGS "${contents}/classes.dex" library_r REGEX "com/example/lib/R")
if(NOT library_r)
  string(APPEND RunCMake_TEST_FAILED
    "classes.dex does not contain the R class of the Android archive\n")
endif()

# The compiled manifest and resources can only be read with aapt2.
set(aapt2)
foreach(sdk IN ITEMS "${CMake_TEST_CPACK_APK_SDK_ROOT}"
    "$ENV{ANDROID_HOME}" "$ENV{ANDROID_SDK_ROOT}")
  if(sdk)
    file(GLOB aapt2 "${sdk}/build-tools/*/aapt2" "${sdk}/build-tools/*/aapt2.exe")
    if(aapt2)
      list(GET aapt2 0 aapt2)
      break()
    endif()
  endif()
endforeach()
if(NOT aapt2)
  string(APPEND RunCMake_TEST_FAILED
    "No aapt2 to check the APK with.  Set CMake_TEST_CPACK_APK_SDK_ROOT.\n")
  return()
endif()

execute_process(COMMAND "${aapt2}" dump badging "${apk}"
  OUTPUT_VARIABLE badging ERROR_VARIABLE badging)
execute_process(COMMAND "${aapt2}" dump xmltree --file AndroidManifest.xml "${apk}"
  OUTPUT_VARIABLE manifest ERROR_VARIABLE manifest)

# A target API level set nowhere is the compile API level.
if(NOT DEFINED expected_target_sdk)
  set(expected_target_sdk 35)
elseif(expected_target_sdk STREQUAL "compile")
  if(badging MATCHES "compileSdkVersion='([0-9]+)'")
    set(expected_target_sdk "${CMAKE_MATCH_1}")
  else()
    string(APPEND RunCMake_TEST_FAILED
      "aapt2 dump badging lacks compileSdkVersion\n")
  endif()
endif()

# Explicit settings replace the values declared by the manifest; the version
# name the manifest leaves out defaults to the package version.
foreach(expected IN ITEMS
    "package: name='com.example.app'"
    "versionCode='42'"
    "versionName='1.2.3'"
    "minSdkVersion:'24'"
    "targetSdkVersion:'${expected_target_sdk}'"
    "uses-permission: name='android.permission.INTERNET'"
    "application-label:'${expected_label}'"
    )
  string(FIND "${badging}" "${expected}" position)
  if(position EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED "aapt2 dump badging lacks ${expected}\n")
  endif()
endforeach()

# The components of the archive's manifest are merged.
foreach(expected IN ITEMS
    "\"com.example.lib.LibService\""
    "\"1 > 0\""
    "\"com.example.lib.LibAlias\""
    "targetActivity(0x01010202)=\"com.example.lib.LibActivity\""
    "\"com.example.lib.LibProvider\""
    "\"com.example.app.lib\""
    "\"com.example.lib.key\""
    "\"com.example.app.self\""
    "\"test-flavor\""
    # The children of components declared again are merged.
    "\"com.example.lib2.key\""
    "\"com.example.lib.Initializer\""
    "\"com.example.lib2.Initializer\""
    )
  string(FIND "${manifest}" "${expected}" position)
  if(position EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED "The manifest lacks ${expected}\n")
  endif()
endforeach()
foreach(unexpected IN ITEMS
    Removed Commented READ_PHONE_STATE applicationId duplicate)
  string(FIND "${manifest}" "${unexpected}" position)
  if(NOT position EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED "The manifest holds ${unexpected}\n")
  endif()
endforeach()
foreach(component IN ITEMS
    com.example.lib.LibProvider androidx.startup.InitializationProvider)
  string(REPLACE "." "\\." component_regex "${component}")
  string(REGEX MATCHALL "\\)=\"${component_regex}\"" declarations
    "${manifest}")
  list(LENGTH declarations count)
  if(NOT count EQUAL 1)
    string(APPEND RunCMake_TEST_FAILED
      "The manifest declares ${component} ${count} times\n")
  endif()
endforeach()

# The archive's `<queries>` is merged at the top level, before
# `<application>`, the `<provider>` it holds included.
string(FIND "${manifest}" "E: queries" queries_position)
string(FIND "${manifest}" "\"com.example.queried\"" queried_position)
string(FIND "${manifest}" "E: application" application_position)
if(queries_position EQUAL -1 OR queried_position LESS queries_position
    OR queried_position GREATER application_position)
  string(APPEND RunCMake_TEST_FAILED
    "The manifest does not hold the archive's <queries> at the top level\n")
endif()

# The uncompressed native libraries are aligned to 16 KB memory pages.
get_filename_component(build_tools "${aapt2}" DIRECTORY)
get_filename_component(build_tools_version "${build_tools}" NAME)
if(build_tools_version VERSION_GREATER_EQUAL 35)
  file(GLOB zipalign "${build_tools}/zipalign" "${build_tools}/zipalign.exe")
  execute_process(COMMAND "${zipalign}" -c -P 16 4 "${apk}"
    RESULT_VARIABLE zipalign_result
    OUTPUT_VARIABLE zipalign_output ERROR_VARIABLE zipalign_output)
  if(NOT zipalign_result EQUAL 0)
    string(APPEND RunCMake_TEST_FAILED
      "The APK is not aligned to 16 KB pages:\n${zipalign_output}\n")
  endif()
endif()

if(RunCMake_TEST_FAILED)
  string(APPEND RunCMake_TEST_FAILED
    "aapt2 dump badging:\n${badging}\n"
    "aapt2 dump xmltree AndroidManifest.xml:\n${manifest}\n")
endif()
