# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Assemble an Android APK from the CPack staging tree by driving the Android
# SDK build tools (aapt2, d8, zipalign, apksigner) and the JDK directly.
# Gradle is deliberately not used.

if(CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CPackAPK.cmake may only be used by CPack internally.")
endif()

function(_cpack_apk_debug)
  if(CPACK_APK_DEBUG)
    message("CPackAPK:Debug: " ${ARGN})
  endif()
endfunction()

# Run a command, failing the package generation with the captured output
# if it does not succeed.
function(_cpack_apk_run description)
  set(_command ${ARGN})
  list(JOIN _command " " _command_text)
  _cpack_apk_debug("Running ${description}: ${_command_text}")
  execute_process(
    COMMAND ${_command}
    WORKING_DIRECTORY "${CPACK_APK_WORK_DIRECTORY}"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE _error
    )
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR
      "CPack APK: ${description} failed with exit code ${_result}.\n"
      "Command: ${_command_text}\n"
      "${_output}${_error}")
  endif()
  if(_output OR _error)
    _cpack_apk_debug("Output of ${description}: ${_output}${_error}")
  endif()
endfunction()

#-----------------------------------------------------------------------------
# Locate the Android SDK
#-----------------------------------------------------------------------------

function(_cpack_apk_find_sdk_root result)
  # An explicitly given SDK is used or rejected, never silently replaced by
  # one found elsewhere.
  if(CPACK_APK_SDK_ROOT)
    if(IS_DIRECTORY "${CPACK_APK_SDK_ROOT}/build-tools")
      file(REAL_PATH "${CPACK_APK_SDK_ROOT}" _real)
      set(${result} "${_real}" PARENT_SCOPE)
    else()
      set(${result} "" PARENT_SCOPE)
    endif()
    return()
  endif()
  set(_candidates)
  foreach(_var IN ITEMS ANDROID_HOME ANDROID_SDK_ROOT)
    if(DEFINED ENV{${_var}})
      list(APPEND _candidates "$ENV{${_var}}")
    endif()
  endforeach()
  # The NDK usually lives inside the SDK, either as <sdk>/ndk/<version>
  # or as <sdk>/ndk-bundle.
  set(_ndk_roots "${CPACK_APK_NDK_ROOT}")
  foreach(_var IN ITEMS ANDROID_NDK_ROOT ANDROID_NDK_HOME ANDROID_NDK)
    if(DEFINED ENV{${_var}})
      list(APPEND _ndk_roots "$ENV{${_var}}")
    endif()
  endforeach()
  foreach(_ndk IN LISTS _ndk_roots)
    if(_ndk)
      list(APPEND _candidates "${_ndk}/../.." "${_ndk}/..")
    endif()
  endforeach()

  foreach(_candidate IN LISTS _candidates)
    if(IS_DIRECTORY "${_candidate}/build-tools")
      file(REAL_PATH "${_candidate}" _real)
      set(${result} "${_real}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${result} "" PARENT_SCOPE)
endfunction()

# Pick the highest version among the sub-directories of `directory` whose
# name matches `regex`, with the version captured in group 1.
function(_cpack_apk_newest_version directory regex result)
  file(GLOB _entries RELATIVE "${directory}" "${directory}/*")
  set(_versions)
  foreach(_entry IN LISTS _entries)
    if(IS_DIRECTORY "${directory}/${_entry}" AND _entry MATCHES "${regex}")
      list(APPEND _versions "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  if(_versions)
    list(SORT _versions COMPARE NATURAL ORDER DESCENDING)
    list(GET _versions 0 _newest)
    set(${result} "${_newest}" PARENT_SCOPE)
  else()
    set(${result} "" PARENT_SCOPE)
  endif()
endfunction()

_cpack_apk_find_sdk_root(_sdk_root)
if(NOT _sdk_root)
  if(CPACK_APK_SDK_ROOT)
    message(FATAL_ERROR
      "CPack APK: CPACK_APK_SDK_ROOT is set to '${CPACK_APK_SDK_ROOT}', "
      "which holds no 'build-tools' directory.  Install the build tools "
      "with the Android SDK manager.")
  endif()
  message(FATAL_ERROR
    "CPack APK: Cannot find the Android SDK.  Set CPACK_APK_SDK_ROOT to the "
    "SDK root directory, or set the ANDROID_HOME environment variable.")
endif()
set(CPACK_APK_SDK_ROOT "${_sdk_root}")
_cpack_apk_debug("Android SDK: ${CPACK_APK_SDK_ROOT}")

if(NOT CPACK_APK_BUILD_TOOLS_VERSION)
  _cpack_apk_newest_version(
    "${CPACK_APK_SDK_ROOT}/build-tools" "^([0-9][0-9.]*)$"
    CPACK_APK_BUILD_TOOLS_VERSION)
  if(NOT CPACK_APK_BUILD_TOOLS_VERSION)
    message(FATAL_ERROR
      "CPack APK: No build tools found in "
      "'${CPACK_APK_SDK_ROOT}/build-tools'.  Install them with the Android "
      "SDK manager or set CPACK_APK_BUILD_TOOLS_VERSION.")
  endif()
endif()
set(CPACK_APK_BUILD_TOOLS_DIRECTORY
  "${CPACK_APK_SDK_ROOT}/build-tools/${CPACK_APK_BUILD_TOOLS_VERSION}")
if(NOT IS_DIRECTORY "${CPACK_APK_BUILD_TOOLS_DIRECTORY}")
  message(FATAL_ERROR
    "CPack APK: Android SDK build tools version "
    "'${CPACK_APK_BUILD_TOOLS_VERSION}' not found at "
    "'${CPACK_APK_BUILD_TOOLS_DIRECTORY}'.")
endif()
_cpack_apk_debug("Build tools: ${CPACK_APK_BUILD_TOOLS_DIRECTORY}")

if(NOT CPACK_APK_COMPILE_SDK_VERSION)
  _cpack_apk_newest_version(
    "${CPACK_APK_SDK_ROOT}/platforms" "^android-([0-9]+)$"
    CPACK_APK_COMPILE_SDK_VERSION)
  if(NOT CPACK_APK_COMPILE_SDK_VERSION)
    message(FATAL_ERROR
      "CPack APK: No platform found in '${CPACK_APK_SDK_ROOT}/platforms'.  "
      "Install one with the Android SDK manager or set "
      "CPACK_APK_COMPILE_SDK_VERSION.")
  endif()
endif()
set(CPACK_APK_ANDROID_JAR
  "${CPACK_APK_SDK_ROOT}/platforms/android-${CPACK_APK_COMPILE_SDK_VERSION}/android.jar")
if(NOT EXISTS "${CPACK_APK_ANDROID_JAR}")
  message(FATAL_ERROR
    "CPack APK: '${CPACK_APK_ANDROID_JAR}' not found.  Install the "
    "'platforms;android-${CPACK_APK_COMPILE_SDK_VERSION}' SDK package or set "
    "CPACK_APK_COMPILE_SDK_VERSION to an installed platform.")
endif()
_cpack_apk_debug("android.jar: ${CPACK_APK_ANDROID_JAR}")

#-----------------------------------------------------------------------------
# Locate the tools
#-----------------------------------------------------------------------------

# `apksigner` and `d8` are wrapper scripts, hence the explicit `.bat` names.
foreach(_tool_spec IN ITEMS
    "AAPT2|aapt2" "D8|d8;d8.bat"
    "ZIPALIGN|zipalign" "APKSIGNER|apksigner;apksigner.bat")
  string(REPLACE "|" ";" _tool_spec "${_tool_spec}")
  list(POP_FRONT _tool_spec _tool_var)
  if(NOT CPACK_APK_${_tool_var}_EXECUTABLE)
    find_program(CPACK_APK_${_tool_var}_EXECUTABLE
      NAMES ${_tool_spec}
      HINTS "${CPACK_APK_BUILD_TOOLS_DIRECTORY}"
      NO_DEFAULT_PATH
      )
  endif()
  if(NOT CPACK_APK_${_tool_var}_EXECUTABLE)
    list(GET _tool_spec 0 _tool_name)
    message(FATAL_ERROR
      "CPack APK: '${_tool_name}' not found in "
      "'${CPACK_APK_BUILD_TOOLS_DIRECTORY}'.  Set "
      "CPACK_APK_${_tool_var}_EXECUTABLE to its location.")
  endif()
endforeach()

# The NDK provides the `llvm-strip`, the `llvm-readelf` and the shared C++
# runtime.
set(_ndk_roots "${CPACK_APK_NDK_ROOT}")
foreach(_var IN ITEMS ANDROID_NDK_ROOT ANDROID_NDK_HOME ANDROID_NDK)
  if(DEFINED ENV{${_var}})
    file(TO_CMAKE_PATH "$ENV{${_var}}" _ndk)
    list(APPEND _ndk_roots "${_ndk}")
  endif()
endforeach()
_cpack_apk_newest_version("${CPACK_APK_SDK_ROOT}/ndk" "^([0-9][0-9.]*)$"
  _ndk_version)
if(_ndk_version)
  list(APPEND _ndk_roots "${CPACK_APK_SDK_ROOT}/ndk/${_ndk_version}")
endif()
list(APPEND _ndk_roots "${CPACK_APK_SDK_ROOT}/ndk-bundle")

# A tool set in `var`, possibly with arguments of its own, is used or
# rejected.  Otherwise, `var` is set to the `tool` of the NDK, if any.
function(_cpack_apk_find_ndk_tool var tool)
  if(${var})
    list(GET ${var} 0 _tool)
    if(NOT IS_ABSOLUTE "${_tool}")
      find_program(_tool_found NAMES "${_tool}" NO_CACHE)
      set(_tool "${_tool_found}")
    endif()
    if(NOT EXISTS "${_tool}" OR IS_DIRECTORY "${_tool}")
      list(GET ${var} 0 _tool)
      message(FATAL_ERROR
        "CPack APK: ${var} is set to '${_tool}', which does not exist.")
    endif()
    return()
  endif()
  foreach(_ndk IN LISTS _ndk_roots)
    if(NOT _ndk)
      continue()
    endif()
    file(GLOB _tools
      "${_ndk}/toolchains/llvm/prebuilt/*/bin/${tool}"
      "${_ndk}/toolchains/llvm/prebuilt/*/bin/${tool}.exe")
    if(_tools)
      list(GET _tools 0 ${var})
      return(PROPAGATE ${var})
    endif()
  endforeach()
endfunction()

# `llvm-strip` of the NDK strips the native libraries, as with the Android
# Gradle plugin.  Without one, they are packaged as they are.
_cpack_apk_find_ndk_tool(CPACK_APK_STRIP_EXECUTABLE llvm-strip)
_cpack_apk_debug("Strip: ${CPACK_APK_STRIP_EXECUTABLE}")

# `llvm-readelf` of the NDK tells which native libraries are linked to the
# shared C++ runtime.  CPack sets CPACK_READELF_EXECUTABLE to CMAKE_READELF,
# which is the one of the NDK when cross-compiling for Android.
_cpack_apk_find_ndk_tool(CPACK_READELF_EXECUTABLE llvm-readelf)
_cpack_apk_debug("Readelf: ${CPACK_READELF_EXECUTABLE}")

# The libraries of the NDK sysroot, `usr/lib/<triple>/`, provide the shared
# C++ runtime.  The `llvm-strip` and `llvm-readelf` of an NDK, such as those
# of a toolchain file, sit next to the sysroot.
set(_sysroots)
foreach(_ndk IN LISTS _ndk_roots)
  if(_ndk)
    file(GLOB _ndk_sysroots "${_ndk}/toolchains/llvm/prebuilt/*/sysroot")
    list(APPEND _sysroots ${_ndk_sysroots})
  endif()
endforeach()
foreach(_tool_var IN ITEMS CPACK_APK_STRIP_EXECUTABLE CPACK_READELF_EXECUTABLE)
  if(${_tool_var})
    list(GET ${_tool_var} 0 _tool)
    get_filename_component(_tool_directory "${_tool}" DIRECTORY)
    list(APPEND _sysroots "${_tool_directory}/../sysroot")
  endif()
endforeach()
set(_sysroot_libraries)
foreach(_sysroot IN LISTS _sysroots)
  if(IS_DIRECTORY "${_sysroot}/usr/lib")
    set(_sysroot_libraries "${_sysroot}/usr/lib")
    break()
  endif()
endforeach()
_cpack_apk_debug("NDK sysroot libraries: ${_sysroot_libraries}")

# The names of the native libraries to keep the debug symbols of.
set(_keep_debug_symbols_regexes)
foreach(_pattern IN LISTS CPACK_APK_KEEP_DEBUG_SYMBOLS)
  string(REGEX REPLACE "([][.+^$()|\\\\{}])" "\\\\\\1" _regex "${_pattern}")
  string(REPLACE "*" ".*" _regex "${_regex}")
  string(REPLACE "?" "." _regex "${_regex}")
  list(APPEND _keep_debug_symbols_regexes "^${_regex}$")
endforeach()

# Settle on the JDK first and have FindJava look in there, so that all the
# Java tools come from the same JDK.  FindJava asks `/usr/libexec/java_home`
# for it only when `APPLE` is set, which it is not when cpack runs.
if(CPACK_APK_JAVA_HOME)
  # The tools run in the work directory.
  file(REAL_PATH "${CPACK_APK_JAVA_HOME}" CPACK_APK_JAVA_HOME)
else()
  if(DEFINED ENV{JAVA_HOME})
    file(TO_CMAKE_PATH "$ENV{JAVA_HOME}" CPACK_APK_JAVA_HOME)
  elseif(CMAKE_HOST_APPLE AND EXISTS "/usr/libexec/java_home")
    execute_process(COMMAND /usr/libexec/java_home
      OUTPUT_VARIABLE CPACK_APK_JAVA_HOME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      )
  endif()
  if(NOT IS_DIRECTORY "${CPACK_APK_JAVA_HOME}")
    set(CPACK_APK_JAVA_HOME)
  endif()
  if(NOT CPACK_APK_JAVA_HOME)
    # The `javac` found on the `PATH` may well be a link into the JDK, such
    # as the `/usr/bin/javac` of the Debian alternatives system.  Unlike
    # `java`, it cannot belong to a JRE lacking the other tools of a JDK.
    find_program(_cpack_apk_javac NAMES javac NO_CACHE)
    if(_cpack_apk_javac)
      file(REAL_PATH "${_cpack_apk_javac}" CPACK_APK_JAVA_HOME)
      get_filename_component(CPACK_APK_JAVA_HOME "${CPACK_APK_JAVA_HOME}"
        DIRECTORY)
      get_filename_component(CPACK_APK_JAVA_HOME "${CPACK_APK_JAVA_HOME}"
        DIRECTORY)
    endif()
  endif()
endif()
if(CPACK_APK_JAVA_HOME)
  set(JAVA_HOME "${CPACK_APK_JAVA_HOME}")
endif()

find_package(Java QUIET COMPONENTS Development)
if(NOT Java_JAVA_EXECUTABLE)
  message(FATAL_ERROR
    "CPack APK: No Java runtime found.  The Android build tools 'd8' and "
    "'apksigner' require a JDK.  Set the JAVA_HOME environment variable or "
    "CPACK_APK_JAVA_HOME.")
endif()
if(NOT CPACK_APK_JAVA_HOME)
  # No JDK then, which the checks below report.
  file(REAL_PATH "${Java_JAVA_EXECUTABLE}" CPACK_APK_JAVA_HOME)
  get_filename_component(CPACK_APK_JAVA_HOME "${CPACK_APK_JAVA_HOME}" DIRECTORY)
  get_filename_component(CPACK_APK_JAVA_HOME "${CPACK_APK_JAVA_HOME}" DIRECTORY)
endif()
_cpack_apk_debug("JAVA_HOME: ${CPACK_APK_JAVA_HOME}")

foreach(_java_tool IN ITEMS jar keytool)
  string(TOUPPER "${_java_tool}" _java_tool_var)
  if(NOT CPACK_APK_${_java_tool_var}_EXECUTABLE)
    find_program(CPACK_APK_${_java_tool_var}_EXECUTABLE
      NAMES ${_java_tool}
      HINTS "${CPACK_APK_JAVA_HOME}/bin"
      NO_DEFAULT_PATH
      )
  endif()
  if(NOT CPACK_APK_${_java_tool_var}_EXECUTABLE)
    message(FATAL_ERROR
      "CPack APK: '${_java_tool}' not found in "
      "'${CPACK_APK_JAVA_HOME}/bin'.  A JDK is required to build an APK.")
  endif()
endforeach()

# The build tool wrapper scripts look at JAVA_HOME to find the JVM.
set(CPACK_APK_ENV "${CMAKE_COMMAND}" -E env "JAVA_HOME=${CPACK_APK_JAVA_HOME}")

#-----------------------------------------------------------------------------
# Collect the inputs
#-----------------------------------------------------------------------------

# Make the relative paths listed in the variable named `var` absolute.  They
# are relative to the directory cpack runs in, but the tools run in the work
# directory.  An unset variable stays unset, so that its default applies.
function(_cpack_apk_absolute_paths var)
  if(NOT DEFINED ${var})
    return()
  endif()
  set(_paths)
  foreach(_path IN LISTS ${var})
    if(NOT IS_ABSOLUTE "${_path}")
      file(REAL_PATH "${_path}" _path)
    endif()
    list(APPEND _paths "${_path}")
  endforeach()
  set(${var} "${_paths}" PARENT_SCOPE)
endfunction()

foreach(_path_var IN ITEMS
    MANIFEST RESOURCE_DIRECTORIES ASSET_DIRECTORIES JAVA_SOURCE_DIRECTORIES
    JARS KEYSTORE MAVEN_CACHE_DIRECTORY)
  _cpack_apk_absolute_paths(CPACK_APK_${_path_var})
endforeach()

set(CPACK_APK_STAGING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}")
# Named after the staging tree, so that no package file name can make the two
# the same directory and have the staged files removed below.
set(CPACK_APK_WORK_DIRECTORY "${CPACK_APK_STAGING_DIRECTORY}.apkbuild")
file(REMOVE_RECURSE "${CPACK_APK_WORK_DIRECTORY}")
file(MAKE_DIRECTORY "${CPACK_APK_WORK_DIRECTORY}")

# Paths of the staging tree that are consumed by the build tools instead of
# being copied into the APK verbatim.
set(_consumed_paths)

# Whether `path`, relative to the staging tree, is one of them.
function(_cpack_apk_is_consumed path result)
  foreach(_consumed IN LISTS _consumed_paths)
    if(_consumed MATCHES "/$")
      # A consumed directory, recorded with a trailing slash.
      string(FIND "${path}" "${_consumed}" _position)
      if(_position EQUAL 0)
        set(${result} TRUE PARENT_SCOPE)
        return()
      endif()
    elseif("${path}" STREQUAL "${_consumed}")
      set(${result} TRUE PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${result} FALSE PARENT_SCOPE)
endfunction()

# These are always handed to the build tools, never copied into the APK
# as they are: `aapt2` puts its own compiled versions of them there.
if(EXISTS "${CPACK_APK_STAGING_DIRECTORY}/AndroidManifest.xml")
  list(APPEND _consumed_paths "AndroidManifest.xml")
  if(NOT CPACK_APK_MANIFEST)
    set(CPACK_APK_MANIFEST "${CPACK_APK_STAGING_DIRECTORY}/AndroidManifest.xml")
  endif()
endif()
if(NOT CPACK_APK_MANIFEST)
  message(FATAL_ERROR
    "CPack APK: No AndroidManifest.xml.  Set CPACK_APK_MANIFEST to its "
    "location, or install it to the root of the packaging tree.")
endif()
if(NOT EXISTS "${CPACK_APK_MANIFEST}")
  message(FATAL_ERROR
    "CPack APK: CPACK_APK_MANIFEST is set to '${CPACK_APK_MANIFEST}', "
    "which does not exist.")
endif()

if(IS_DIRECTORY "${CPACK_APK_STAGING_DIRECTORY}/res")
  list(APPEND _consumed_paths "res/")
  if(NOT DEFINED CPACK_APK_RESOURCE_DIRECTORIES)
    set(CPACK_APK_RESOURCE_DIRECTORIES "${CPACK_APK_STAGING_DIRECTORY}/res")
  endif()
endif()

if(IS_DIRECTORY "${CPACK_APK_STAGING_DIRECTORY}/assets")
  list(APPEND _consumed_paths "assets/")
  if(NOT DEFINED CPACK_APK_ASSET_DIRECTORIES)
    set(CPACK_APK_ASSET_DIRECTORIES "${CPACK_APK_STAGING_DIRECTORY}/assets")
  endif()
endif()

# Java archives staged for installation are compiled into the APK's bytecode
# rather than packaged as-is.
file(GLOB_RECURSE _staged_jars
  RELATIVE "${CPACK_APK_STAGING_DIRECTORY}"
  "${CPACK_APK_STAGING_DIRECTORY}/*.jar" "${CPACK_APK_STAGING_DIRECTORY}/*.aar")
foreach(_staged_jar IN LISTS _staged_jars)
  # Unless they are resources: an archive below `res/` or `assets/` is data
  # the application reads at runtime, not code to compile into it.
  _cpack_apk_is_consumed("${_staged_jar}" _is_resource)
  if(_is_resource)
    continue()
  endif()
  list(APPEND _consumed_paths "${_staged_jar}")
  list(APPEND CPACK_APK_JARS "${CPACK_APK_STAGING_DIRECTORY}/${_staged_jar}")
endforeach()

if(CPACK_APK_MAVEN_DEPENDENCIES)
  include("${CMAKE_CURRENT_LIST_DIR}/CPackAPKMaven.cmake")
  _cpack_apk_maven_resolve(_maven_archives)
  list(APPEND CPACK_APK_JARS ${_maven_archives})
endif()
# A staged archive may be listed explicitly as well, and `d8` rejects the
# classes it would then see twice.
list(REMOVE_DUPLICATES CPACK_APK_JARS)

# An Android archive is a zip holding `classes.jar` and, optionally, its own
# manifest, resources, assets and native libraries.  Unpack the parts we know
# how to consume.
set(_jars)
set(_aar_index 0)
set(_aar_manifests)
set(_aar_resource_directories)
set(_aar_asset_directories)
set(_aar_native_libraries)
foreach(_jar IN LISTS CPACK_APK_JARS)
  if(NOT EXISTS "${_jar}")
    message(FATAL_ERROR
      "CPack APK: '${_jar}' listed in CPACK_APK_JARS does not exist.")
  endif()
  if(NOT _jar MATCHES "\\.aar$")
    list(APPEND _jars "${_jar}")
    continue()
  endif()
  # Archives from different directories may well share a file name.
  get_filename_component(_aar_name "${_jar}" NAME_WLE)
  set(_aar_dir "${CPACK_APK_WORK_DIRECTORY}/aar/${_aar_index}-${_aar_name}")
  math(EXPR _aar_index "${_aar_index} + 1")
  file(ARCHIVE_EXTRACT INPUT "${_jar}" DESTINATION "${_aar_dir}")
  if(EXISTS "${_aar_dir}/classes.jar")
    list(APPEND _jars "${_aar_dir}/classes.jar")
  endif()
  file(GLOB _aar_libs "${_aar_dir}/libs/*.jar")
  list(APPEND _jars ${_aar_libs})
  if(EXISTS "${_aar_dir}/AndroidManifest.xml")
    list(APPEND _aar_manifests "${_aar_dir}/AndroidManifest.xml")
  endif()
  if(IS_DIRECTORY "${_aar_dir}/res")
    list(APPEND _aar_resource_directories "${_aar_dir}/res")
  endif()
  if(IS_DIRECTORY "${_aar_dir}/assets")
    list(APPEND _aar_asset_directories "${_aar_dir}/assets")
  endif()
  file(GLOB _aar_jni RELATIVE "${_aar_dir}/jni" "${_aar_dir}/jni/*/*.so")
  foreach(_aar_library IN LISTS _aar_jni)
    list(APPEND _aar_native_libraries "${_aar_dir}/jni/${_aar_library}")
  endforeach()
endforeach()
set(CPACK_APK_JARS "${_jars}")

# The application overrides the resources and assets of the archives it
# bundles, so theirs come first.
list(PREPEND CPACK_APK_RESOURCE_DIRECTORIES ${_aar_resource_directories})
list(PREPEND CPACK_APK_ASSET_DIRECTORIES ${_aar_asset_directories})

set(_java_sources)
foreach(_java_dir IN LISTS CPACK_APK_JAVA_SOURCE_DIRECTORIES)
  if(NOT IS_DIRECTORY "${_java_dir}")
    message(FATAL_ERROR
      "CPack APK: '${_java_dir}' listed in "
      "CPACK_APK_JAVA_SOURCE_DIRECTORIES is not a directory.")
  endif()
  file(GLOB_RECURSE _dir_sources "${_java_dir}/*.java")
  list(APPEND _java_sources ${_dir_sources})
endforeach()

if(NOT DEFINED CPACK_APK_NO_COMPRESS_EXTENSIONS)
  # Android can map uncompressed native libraries straight out of the APK,
  # which is required when the manifest sets `extractNativeLibs` to false.
  set(CPACK_APK_NO_COMPRESS_EXTENSIONS ".so")
endif()

#-----------------------------------------------------------------------------
# Preprocess the manifest
#-----------------------------------------------------------------------------

# The manifest is edited as text rather than parsed as XML.  That is enough
# for the attributes and elements touched below, which are all well known.
set(_xml_space "[ \t\r\n]")
set(_xml_value "(\"([^\"]*)\"|'([^']*)')")
set(_xml_assignment "${_xml_space}*=${_xml_space}*${_xml_value}")
# What follows the element name of a start tag, attributes included.  A `>`
# may appear in a quoted attribute value without ending the tag.
set(_xml_tag_rest
  "(${_xml_space}[^\"'>]*((\"[^\"]*\"|'[^']*')[^\"'>]*)*)?/?>")

# Escape `value` for an attribute.
function(_cpack_apk_xml_escape value result)
  string(REPLACE "&" "&amp;" value "${value}")
  string(REPLACE "<" "&lt;" value "${value}")
  string(REPLACE ">" "&gt;" value "${value}")
  string(REPLACE "\"" "&quot;" value "${value}")
  set(${result} "${value}" PARENT_SCOPE)
endfunction()

# Replace the first occurrence of `old` in the variable named `content_var`.
function(_cpack_apk_replace_first content_var old new)
  string(FIND "${${content_var}}" "${old}" _position)
  if(_position EQUAL -1)
    return()
  endif()
  string(LENGTH "${old}" _length)
  math(EXPR _after "${_position} + ${_length}")
  string(SUBSTRING "${${content_var}}" 0 ${_position} _head)
  string(SUBSTRING "${${content_var}}" ${_after} -1 _tail)
  set(${content_var} "${_head}${new}${_tail}" PARENT_SCOPE)
endfunction()

# Insert `text` into the variable named `content_var` in front of the first,
# or with REVERSE the last, occurrence of `marker`.
function(_cpack_apk_insert_before content_var marker text)
  string(FIND "${${content_var}}" "${marker}" _position ${ARGN})
  if(_position EQUAL -1)
    message(FATAL_ERROR "CPack APK: No '${marker}' in the manifest.")
  endif()
  string(SUBSTRING "${${content_var}}" 0 ${_position} _head)
  string(SUBSTRING "${${content_var}}" ${_position} -1 _tail)
  set(${content_var} "${_head}${text}${_tail}" PARENT_SCOPE)
endfunction()

# Get the value of `attribute` from the start tag `tag`.
function(_cpack_apk_tag_attribute tag attribute result)
  if(tag MATCHES "${_xml_space}${attribute}${_xml_assignment}")
    set(${result} "${CMAKE_MATCH_2}${CMAKE_MATCH_3}" PARENT_SCOPE)
  else()
    set(${result} "" PARENT_SCOPE)
  endif()
endfunction()

# Set `attribute` of the start tag held by the variable named `tag_var`.
function(_cpack_apk_set_tag_attribute tag_var attribute value)
  _cpack_apk_xml_escape("${value}" value)
  set(_tag "${${tag_var}}")
  set(_assignment " ${attribute}=\"${value}\"")
  if(_tag MATCHES "${_xml_space}${attribute}${_xml_assignment}")
    _cpack_apk_replace_first(_tag "${CMAKE_MATCH_0}" "${_assignment}")
  else()
    string(REGEX MATCH "^<[^ \t\r\n/>]+" _element "${_tag}")
    string(LENGTH "${_element}" _length)
    string(SUBSTRING "${_tag}" ${_length} -1 _tail)
    set(_tag "${_element}${_assignment}${_tail}")
  endif()
  set(${tag_var} "${_tag}" PARENT_SCOPE)
endfunction()

# Set `attribute` of the first `element` of the manifest held by the
# variable named `content_var`.  A missing `<uses-sdk>` is added.
function(_cpack_apk_set_manifest_attribute content_var element attribute value)
  set(_content "${${content_var}}")
  if(NOT _content MATCHES "<${element}${_xml_tag_rest}")
    if(NOT element STREQUAL "uses-sdk"
        OR NOT _content MATCHES "<manifest${_xml_tag_rest}")
      message(FATAL_ERROR "CPack APK: No <${element}> in the manifest.")
    endif()
    _cpack_apk_replace_first(_content "${CMAKE_MATCH_0}"
      "${CMAKE_MATCH_0}\n  <uses-sdk/>")
    string(REGEX MATCH "<${element}${_xml_tag_rest}" _tag "${_content}")
  else()
    set(_tag "${CMAKE_MATCH_0}")
  endif()
  set(_new_tag "${_tag}")
  _cpack_apk_set_tag_attribute(_new_tag "${attribute}" "${value}")
  _cpack_apk_replace_first(_content "${_tag}" "${_new_tag}")
  set(${content_var} "${_content}" PARENT_SCOPE)
endfunction()

# Resolve the Gradle-style `${<name>}` placeholders in the variable named
# `content_var`.
function(_cpack_apk_apply_placeholders content_var)
  set(_content "${${content_var}}")
  foreach(_placeholder IN LISTS CPACK_APK_MANIFEST_PLACEHOLDERS)
    if(NOT _placeholder MATCHES "^([^=]+)=(.*)$")
      message(FATAL_ERROR
        "CPack APK: CPACK_APK_MANIFEST_PLACEHOLDERS entry '${_placeholder}' "
        "is not of the form <name>=<value>.")
    endif()
    string(REPLACE "\${${CMAKE_MATCH_1}}" "${CMAKE_MATCH_2}"
      _content "${_content}")
  endforeach()
  set(${content_var} "${_content}" PARENT_SCOPE)
endfunction()

function(_cpack_apk_strip_xml_comments content_var)
  set(_content "${${content_var}}")
  while(TRUE)
    string(FIND "${_content}" "<!--" _start)
    if(_start EQUAL -1)
      break()
    endif()
    string(SUBSTRING "${_content}" ${_start} -1 _comment)
    string(FIND "${_comment}" "-->" _length)
    if(_length EQUAL -1)
      message(FATAL_ERROR "CPack APK: Unterminated comment in a manifest.")
    endif()
    math(EXPR _length "${_length} + 3")
    string(SUBSTRING "${_comment}" 0 ${_length} _comment)
    _cpack_apk_replace_first(_content "${_comment}" "")
  endwhile()
  set(${content_var} "${_content}" PARENT_SCOPE)
endfunction()

# The elements of a library manifest merged into the application manifest,
# either at the top level or into `<application>`.  A `<queries>` element
# is taken as a whole, so that the `<provider>` it may hold is not taken for
# a component.
set(_manifest_top_level_elements
  permission uses-permission uses-permission-sdk-23 uses-feature queries)
set(_manifest_component_elements
  activity activity-alias service receiver provider)
set(_manifest_application_elements
  ${_manifest_component_elements}
  meta-data uses-library uses-native-library)

# Qualify the class name `name` found in a manifest of `package`, the way
# Android does: a name starting with a dot, or holding none, is relative.
function(_cpack_apk_qualify_class name package result)
  if(name MATCHES "^\\.")
    set(name "${package}${name}")
  elseif(name AND NOT name MATCHES "\\.")
    set(name "${package}.${name}")
  endif()
  set(${result} "${name}" PARENT_SCOPE)
endfunction()

# Drop the elements marked `tools:node="remove"` from inside the element held
# by the variable named `snippet_var`.
function(_cpack_apk_drop_removed_children snippet_var)
  set(_rest "${${snippet_var}}")
  set(_kept)
  # The element itself is looked at by the caller.
  if(_rest MATCHES "^<[^ \t\r\n/>]+${_xml_tag_rest}")
    string(LENGTH "${CMAKE_MATCH_0}" _length)
    string(SUBSTRING "${_rest}" 0 ${_length} _kept)
    string(SUBSTRING "${_rest}" ${_length} -1 _rest)
  endif()
  while(_rest MATCHES "<([A-Za-z][^ \t\r\n/>]*)${_xml_tag_rest}")
    set(_element "${CMAKE_MATCH_1}")
    set(_tag "${CMAKE_MATCH_0}")
    string(FIND "${_rest}" "${_tag}" _start)
    string(LENGTH "${_tag}" _length)
    math(EXPR _end "${_start} + ${_length}")
    string(SUBSTRING "${_rest}" 0 ${_start} _before)
    string(SUBSTRING "${_rest}" ${_end} -1 _rest)
    _cpack_apk_tag_attribute("${_tag}" "tools:node" _node)
    if(NOT _node MATCHES "^remove")
      string(APPEND _kept "${_before}${_tag}")
      continue()
    endif()
    # Drop the indentation of the removed element along with it.
    string(REGEX REPLACE "\n[ \t]*$" "" _before "${_before}")
    string(APPEND _kept "${_before}")
    if(NOT _tag MATCHES "/>$")
      string(FIND "${_rest}" "</${_element}>" _end)
      if(_end EQUAL -1)
        message(FATAL_ERROR
          "CPack APK: No </${_element}> for '${_tag}' in a manifest.")
      endif()
      string(LENGTH "</${_element}>" _length)
      math(EXPR _end "${_end} + ${_length}")
      string(SUBSTRING "${_rest}" ${_end} -1 _rest)
    endif()
  endwhile()
  set(${snippet_var} "${_kept}${_rest}" PARENT_SCOPE)
endfunction()

# Set `result` to the `<offset>:<length>` in `snippet` of each child element
# of the element it holds.
function(_cpack_apk_child_elements snippet result)
  set(_children)
  if(NOT snippet MATCHES "^<[^ \t\r\n/>]+${_xml_tag_rest}")
    set(${result} "" PARENT_SCOPE)
    return()
  endif()
  set(_tag "${CMAKE_MATCH_0}")
  if(_tag MATCHES "/>$")
    set(${result} "" PARENT_SCOPE)
    return()
  endif()
  string(LENGTH "${_tag}" _offset)
  string(SUBSTRING "${snippet}" ${_offset} -1 _rest)
  # The end tag of the element itself ends the walk.
  while(_rest MATCHES "^([^<]*)<([A-Za-z][^ \t\r\n/>]*)${_xml_tag_rest}")
    set(_element "${CMAKE_MATCH_2}")
    string(LENGTH "${CMAKE_MATCH_1}" _space)
    string(LENGTH "${CMAKE_MATCH_0}" _length)
    if(CMAKE_MATCH_0 MATCHES "/>$")
      math(EXPR _length "${_length} - ${_space}")
    else()
      # Elements of the same kind do not nest, so the first end tag is theirs.
      string(FIND "${_rest}" "</${_element}>" _end)
      if(_end EQUAL -1)
        message(FATAL_ERROR
          "CPack APK: No </${_element}> for <${_element}> in a manifest.")
      endif()
      string(LENGTH "</${_element}>" _length)
      math(EXPR _length "${_end} + ${_length} - ${_space}")
    endif()
    math(EXPR _start "${_offset} + ${_space}")
    list(APPEND _children "${_start}:${_length}")
    math(EXPR _consumed "${_space} + ${_length}")
    math(EXPR _offset "${_offset} + ${_consumed}")
    string(SUBSTRING "${_rest}" ${_consumed} -1 _rest)
  endwhile()
  set(${result} "${_children}" PARENT_SCOPE)
endfunction()

# Set `result` to the key under which the child element `child` of a
# component is merged: its `android:name`, or else its content.
function(_cpack_apk_child_key child result)
  string(REGEX MATCH "^<([^ \t\r\n/>]+)${_xml_tag_rest}" _tag "${child}")
  set(_element "${CMAKE_MATCH_1}")
  _cpack_apk_tag_attribute("${_tag}" "android:name" _name)
  if(_name)
    string(MD5 _key "${_element}:${_name}")
  else()
    # Such as an `<intent-filter>`, which archives may indent differently.
    string(REGEX REPLACE
      "${_xml_space}tools:[A-Za-z]+${_xml_assignment}" "" child "${child}")
    string(REGEX REPLACE ">${_xml_space}+" ">" child "${child}")
    string(REGEX REPLACE "${_xml_space}+<" "<" child "${child}")
    string(REGEX REPLACE "${_xml_space}+" " " child "${child}")
    string(MD5 _key "${child}")
  endif()
  set(${result} "${_key}" PARENT_SCOPE)
endfunction()

# Append `child` to the content of the element held by the variable named
# `snippet_var`.
function(_cpack_apk_append_child snippet_var child)
  set(_snippet "${${snippet_var}}")
  string(REGEX MATCH "^<([^ \t\r\n/>]+)${_xml_tag_rest}" _tag "${_snippet}")
  set(_element "${CMAKE_MATCH_1}")
  if(_tag MATCHES "/>$")
    string(REGEX REPLACE "${_xml_space}*/>$" ">" _start_tag "${_tag}")
    _cpack_apk_replace_first(_snippet "${_tag}"
      "${_start_tag}\n  ${child}\n</${_element}>")
  else()
    string(FIND "${_snippet}" "</${_element}>" _end REVERSE)
    string(SUBSTRING "${_snippet}" 0 ${_end} _head)
    string(SUBSTRING "${_snippet}" ${_end} -1 _tail)
    set(_indent)
    if(_head MATCHES "\n([ \t]*)$")
      set(_indent "${CMAKE_MATCH_1}")
    endif()
    set(_snippet "${_head}  ${child}\n${_indent}${_tail}")
  endif()
  set(${snippet_var} "${_snippet}" PARENT_SCOPE)
endfunction()

# Merge the child elements of the component `snippet` into the component
# already declared under `key`, as the Gradle manifest merger does, unless
# that one was removed or replaces the others.  The children declared under
# the same `android:name` are left out.
function(_cpack_apk_merge_children key snippet)
  get_property(_target GLOBAL PROPERTY _cpack_apk_manifest_${key})
  get_property(_closed GLOBAL PROPERTY _cpack_apk_manifest_closed_${key})
  if(_target STREQUAL "" OR _closed)
    return()
  endif()
  get_property(_child_keys GLOBAL PROPERTY _cpack_apk_manifest_children_${key})
  _cpack_apk_child_elements("${snippet}" _children)
  foreach(_child IN LISTS _children)
    string(REPLACE ":" ";" _child "${_child}")
    list(GET _child 0 _offset)
    list(GET _child 1 _length)
    string(SUBSTRING "${snippet}" ${_offset} ${_length} _child)
    _cpack_apk_child_key("${_child}" _child_key)
    if(_child_key IN_LIST _child_keys)
      continue()
    endif()
    # A child marked for removal still keeps those of the same name out.
    list(APPEND _child_keys "${_child_key}")
    string(REGEX MATCH "^<[^ \t\r\n/>]+${_xml_tag_rest}" _tag "${_child}")
    _cpack_apk_tag_attribute("${_tag}" "tools:node" _node)
    if(_node MATCHES "^remove")
      continue()
    endif()
    _cpack_apk_drop_removed_children(_child)
    string(REGEX REPLACE
      "${_xml_space}tools:[A-Za-z]+${_xml_assignment}"
      "" _child "${_child}")
    _cpack_apk_append_child(_target "${_child}")
  endforeach()
  set_property(GLOBAL PROPERTY _cpack_apk_manifest_${key} "${_target}")
  set_property(GLOBAL PROPERTY
    _cpack_apk_manifest_children_${key} "${_child_keys}")
endfunction()

# Walk the mergeable elements of `content`, a manifest of `package`.
# Each one not in `_manifest_keys` yet is added to it and, unless
# `collect_only` is set, to `_manifest_top_level_keys` or
# `_manifest_application_keys`, in the calling scope, with its text in the
# global property `_cpack_apk_manifest_<key>`.  The child elements of a
# component already in `_manifest_keys` are merged into it instead.  With
# `collect_only`, `content` without the elements marked
# `tools:node="remove"` is stored in `_manifest_kept`, and the components
# it keeps are added to `_manifest_kept_keys`, so that the children merged
# into them can be written back.
function(_cpack_apk_manifest_elements content package collect_only)
  set(_keys "${_manifest_keys}")
  set(_kept)
  set(_kept_keys "${_manifest_kept_keys}")
  set(_top_level "${_manifest_top_level_keys}")
  set(_application "${_manifest_application_keys}")
  list(JOIN _manifest_top_level_elements "|" _names)
  list(JOIN _manifest_application_elements "|" _application_names)
  string(APPEND _names "|${_application_names}")

  set(_rest "${content}")
  while(_rest MATCHES "<(${_names})[ \t\r\n/>]")
    set(_element "${CMAKE_MATCH_1}")
    string(FIND "${_rest}" "${CMAKE_MATCH_0}" _start)
    string(SUBSTRING "${_rest}" 0 ${_start} _before)
    string(APPEND _kept "${_before}")
    string(SUBSTRING "${_rest}" ${_start} -1 _rest)
    if(NOT _rest MATCHES "^<${_element}${_xml_tag_rest}")
      message(FATAL_ERROR "CPack APK: Unterminated <${_element}> in a manifest.")
    endif()
    set(_tag "${CMAKE_MATCH_0}")
    string(LENGTH "${_tag}" _tag_length)
    if(_tag MATCHES "/>$")
      set(_length ${_tag_length})
    else()
      # Elements of the same kind do not nest, so the first end tag is theirs.
      string(FIND "${_rest}" "</${_element}>" _length)
      if(_length EQUAL -1)
        message(FATAL_ERROR
          "CPack APK: No </${_element}> for '${_tag}' in a manifest.")
      endif()
      string(LENGTH "</${_element}>" _end_tag_length)
      math(EXPR _length "${_length} + ${_end_tag_length}")
    endif()
    string(SUBSTRING "${_rest}" 0 ${_length} _snippet)
    string(SUBSTRING "${_rest}" ${_length} -1 _rest)
    set(_full_snippet "${_snippet}")
    _cpack_apk_drop_removed_children(_snippet)

    _cpack_apk_tag_attribute("${_tag}" "android:name" _name)
    # Component class names may be relative to the package of the manifest.
    set(_component FALSE)
    if(_element IN_LIST _manifest_component_elements)
      set(_component TRUE)
      if(package)
        _cpack_apk_qualify_class("${_name}" "${package}" _name)
      endif()
    endif()
    _cpack_apk_tag_attribute("${_tag}" "tools:node" _node)
    if(collect_only)
      if(_node MATCHES "^remove")
        # Drop the indentation of the removed element along with it.
        string(REGEX REPLACE "\n[ \t]*$" "" _kept "${_kept}")
      else()
        string(APPEND _kept "${_snippet}")
      endif()
    endif()
    if(_name)
      string(MD5 _key "${_element}:${_name}")
    else()
      string(MD5 _key "${_snippet}")
    endif()
    if(_key IN_LIST _keys)
      # Components declared again, such as the `InitializationProvider` of
      # AndroidX Startup, contribute their children.
      if(_component AND _name AND NOT collect_only
          AND NOT _node MATCHES "^remove")
        _cpack_apk_merge_children("${_key}" "${_full_snippet}")
      endif()
      continue()
    endif()
    # An element marked for removal still keeps those of the same name out.
    list(APPEND _keys "${_key}")
    if(_component AND _name)
      # The children the element declares, or removes, keep those of the
      # same name out.
      set(_child_keys)
      _cpack_apk_child_elements("${_full_snippet}" _children)
      foreach(_child IN LISTS _children)
        string(REPLACE ":" ";" _child "${_child}")
        list(GET _child 0 _offset)
        list(GET _child 1 _length)
        string(SUBSTRING "${_full_snippet}" ${_offset} ${_length} _child)
        _cpack_apk_child_key("${_child}" _child_key)
        list(APPEND _child_keys "${_child_key}")
      endforeach()
      set_property(GLOBAL PROPERTY
        _cpack_apk_manifest_children_${_key} "${_child_keys}")
      set(_closed FALSE)
      if(_node MATCHES "^(remove|replace)")
        set(_closed TRUE)
      endif()
      set_property(GLOBAL PROPERTY _cpack_apk_manifest_closed_${_key} ${_closed})
    endif()
    if(_node MATCHES "^remove")
      continue()
    endif()
    if(collect_only)
      if(_component AND _name)
        set_property(GLOBAL PROPERTY _cpack_apk_manifest_${_key} "${_snippet}")
        set_property(GLOBAL PROPERTY
          _cpack_apk_manifest_kept_${_key} "${_snippet}")
        list(APPEND _kept_keys "${_key}")
      endif()
      continue()
    endif()

    # Merged into the application manifest, relative class names would
    # resolve against the package of the application.
    if(_component AND package)
      set(_new_tag "${_tag}")
      foreach(_class_attribute IN ITEMS
          android:name android:targetActivity android:parentActivityName)
        _cpack_apk_tag_attribute("${_tag}" "${_class_attribute}" _class)
        _cpack_apk_qualify_class("${_class}" "${package}" _qualified_class)
        if(NOT _qualified_class STREQUAL _class)
          _cpack_apk_set_tag_attribute(_new_tag
            "${_class_attribute}" "${_qualified_class}")
        endif()
      endforeach()
      _cpack_apk_replace_first(_snippet "${_tag}" "${_new_tag}")
    endif()
    # The merge rules the Gradle manifest merger follows are not applied, and
    # the application manifest need not declare their namespace.
    string(REGEX REPLACE
      "${_xml_space}tools:[A-Za-z]+${_xml_assignment}"
      "" _snippet "${_snippet}")
    set_property(GLOBAL PROPERTY _cpack_apk_manifest_${_key} "${_snippet}")
    if(_element IN_LIST _manifest_top_level_elements)
      list(APPEND _top_level "${_key}")
    else()
      list(APPEND _application "${_key}")
    endif()
  endwhile()

  string(APPEND _kept "${_rest}")
  set(_manifest_kept "${_kept}" PARENT_SCOPE)
  set(_manifest_kept_keys "${_kept_keys}" PARENT_SCOPE)
  set(_manifest_keys "${_keys}" PARENT_SCOPE)
  set(_manifest_top_level_keys "${_top_level}" PARENT_SCOPE)
  set(_manifest_application_keys "${_application}" PARENT_SCOPE)
endfunction()

file(READ "${CPACK_APK_MANIFEST}" _manifest_content)
_cpack_apk_apply_placeholders(_manifest_content)
_cpack_apk_strip_xml_comments(_manifest_content)
if(NOT _manifest_content MATCHES "<manifest${_xml_tag_rest}")
  message(FATAL_ERROR
    "CPack APK: '${CPACK_APK_MANIFEST}' holds no <manifest> element.")
endif()
# Manifests of projects built with Gradle leave the package to the build
# script, as its `namespace`.
if(NOT "${CPACK_APK_PACKAGE}" STREQUAL "")
  _cpack_apk_set_manifest_attribute(_manifest_content
    manifest package "${CPACK_APK_PACKAGE}")
  set(_package "${CPACK_APK_PACKAGE}")
else()
  string(REGEX MATCH "<manifest${_xml_tag_rest}" _manifest_tag
    "${_manifest_content}")
  _cpack_apk_tag_attribute("${_manifest_tag}" "package" _package)
  if(_package STREQUAL "")
    message(FATAL_ERROR
      "CPack APK: The <manifest> of '${CPACK_APK_MANIFEST}' has no 'package' "
      "attribute.  Set CPACK_APK_PACKAGE to the package of the application.")
  endif()
endif()

# Unless set explicitly, `${applicationId}` is the package of the application,
# as with Gradle.
if(_package AND NOT CPACK_APK_MANIFEST_PLACEHOLDERS MATCHES "(^|;)applicationId=")
  list(APPEND CPACK_APK_MANIFEST_PLACEHOLDERS "applicationId=${_package}")
  string(REPLACE "\${applicationId}" "${_package}"
    _manifest_content "${_manifest_content}")
endif()

# The elements the application manifest marks `tools:node="remove"` are
# not packaged, and keep those of the same name in the archives out.
set(_manifest_keys)
set(_manifest_kept_keys)
set(_manifest_top_level_keys)
set(_manifest_application_keys)
_cpack_apk_manifest_elements("${_manifest_content}" "${_package}" TRUE)
set(_manifest_content "${_manifest_kept}")

# Merge the manifests of the Android archives.  Their code, resources and
# native libraries are packaged, so the components, permissions and features
# they declare must be as well.
set(_aar_packages)
if(_aar_manifests)
  foreach(_aar_manifest IN LISTS _aar_manifests)
    file(READ "${_aar_manifest}" _aar_manifest_content)
    _cpack_apk_apply_placeholders(_aar_manifest_content)
    _cpack_apk_strip_xml_comments(_aar_manifest_content)
    set(_aar_package)
    if(_aar_manifest_content MATCHES "<manifest${_xml_tag_rest}")
      _cpack_apk_tag_attribute("${CMAKE_MATCH_0}" "package" _aar_package)
    endif()
    if(_aar_package AND NOT _aar_package STREQUAL _package)
      list(APPEND _aar_packages "${_aar_package}")
    endif()
    _cpack_apk_manifest_elements(
      "${_aar_manifest_content}" "${_aar_package}" FALSE)
  endforeach()
  list(REMOVE_DUPLICATES _aar_packages)

  # The components of the application manifest the archives contributed
  # children to.
  foreach(_key IN LISTS _manifest_kept_keys)
    get_property(_kept GLOBAL PROPERTY _cpack_apk_manifest_kept_${_key})
    get_property(_merged GLOBAL PROPERTY _cpack_apk_manifest_${_key})
    if(NOT _merged STREQUAL _kept)
      _cpack_apk_replace_first(_manifest_content "${_kept}" "${_merged}")
    endif()
  endforeach()
  set(_manifest_top_level)
  foreach(_key IN LISTS _manifest_top_level_keys)
    get_property(_snippet GLOBAL PROPERTY _cpack_apk_manifest_${_key})
    string(APPEND _manifest_top_level "  ${_snippet}\n")
  endforeach()
  set(_manifest_application)
  foreach(_key IN LISTS _manifest_application_keys)
    get_property(_snippet GLOBAL PROPERTY _cpack_apk_manifest_${_key})
    string(APPEND _manifest_application "    ${_snippet}\n")
  endforeach()

  if(_manifest_top_level)
    if(_manifest_content MATCHES "<application[ \t\r\n/>]")
      # Keep the indentation of `<application>` for the first element.
      string(STRIP "${_manifest_top_level}" _manifest_top_level)
      _cpack_apk_insert_before(_manifest_content "${CMAKE_MATCH_0}"
        "${_manifest_top_level}\n  ")
    else()
      _cpack_apk_insert_before(_manifest_content "</manifest>"
        "${_manifest_top_level}" REVERSE)
    endif()
  endif()
  if(_manifest_application)
    if(_manifest_content MATCHES "<application${_xml_tag_rest}")
      set(_application_tag "${CMAKE_MATCH_0}")
      if(_application_tag MATCHES "^(.*)/>$")
        _cpack_apk_replace_first(_manifest_content "${_application_tag}"
          "${CMAKE_MATCH_1}>\n${_manifest_application}  </application>")
      else()
        _cpack_apk_insert_before(_manifest_content "</application>"
          "\n${_manifest_application}  ")
      endif()
    else()
      _cpack_apk_insert_before(_manifest_content "</manifest>"
        "  <application>\n${_manifest_application}  </application>\n"
        REVERSE)
    endif()
  endif()
endif()
foreach(_key IN LISTS _manifest_keys)
  foreach(_property IN ITEMS
      _cpack_apk_manifest_${_key}
      _cpack_apk_manifest_kept_${_key}
      _cpack_apk_manifest_children_${_key}
      _cpack_apk_manifest_closed_${_key})
    set_property(GLOBAL PROPERTY ${_property})
  endforeach()
endforeach()

# `aapt2` only fills in the versions and API levels the manifest leaves out,
# so the ones set explicitly are written into the manifest.
foreach(_attribute_spec IN ITEMS
    "VERSION_CODE|manifest|android:versionCode"
    "VERSION_NAME|manifest|android:versionName"
    "MIN_SDK_VERSION|uses-sdk|android:minSdkVersion"
    "TARGET_SDK_VERSION|uses-sdk|android:targetSdkVersion")
  string(REPLACE "|" ";" _attribute_spec "${_attribute_spec}")
  list(GET _attribute_spec 0 _attribute_var)
  list(GET _attribute_spec 1 _attribute_element)
  list(GET _attribute_spec 2 _attribute)
  if(NOT "${CPACK_APK_${_attribute_var}}" STREQUAL "")
    _cpack_apk_set_manifest_attribute(_manifest_content
      "${_attribute_element}" "${_attribute}" "${CPACK_APK_${_attribute_var}}")
  endif()
endforeach()

# `d8` needs to know the minimum API level as well, wherever it comes from.
set(_min_sdk_version)
if(_manifest_content MATCHES "<uses-sdk${_xml_tag_rest}")
  _cpack_apk_tag_attribute("${CMAKE_MATCH_0}" "android:minSdkVersion"
    _min_sdk_version)
  if(NOT _min_sdk_version MATCHES "^[0-9]+$")
    set(_min_sdk_version)
  endif()
endif()

set(CPACK_APK_MANIFEST "${CPACK_APK_WORK_DIRECTORY}/AndroidManifest.xml")
file(WRITE "${CPACK_APK_MANIFEST}" "${_manifest_content}")
_cpack_apk_debug("Manifest: ${CPACK_APK_MANIFEST}")

#-----------------------------------------------------------------------------
# Compile and link the resources into the base APK
#-----------------------------------------------------------------------------

set(_compiled_resources)
set(_resource_index 0)
foreach(_resource_dir IN LISTS CPACK_APK_RESOURCE_DIRECTORIES)
  if(NOT IS_DIRECTORY "${_resource_dir}")
    message(FATAL_ERROR
      "CPack APK: '${_resource_dir}' listed in "
      "CPACK_APK_RESOURCE_DIRECTORIES is not a directory.")
  endif()
  set(_compiled "${CPACK_APK_WORK_DIRECTORY}/resources-${_resource_index}.zip")
  _cpack_apk_run("aapt2 compile of '${_resource_dir}'"
    "${CPACK_APK_AAPT2_EXECUTABLE}" compile --dir "${_resource_dir}"
    -o "${_compiled}")
  # Each directory after the first overlays the ones before it: a resource
  # they share takes its value from the later one instead of conflicting.
  if(_compiled_resources)
    list(APPEND _compiled_resources -R)
  endif()
  list(APPEND _compiled_resources "${_compiled}")
  math(EXPR _resource_index "${_resource_index} + 1")
endforeach()

set(_link_options)
foreach(_extension IN LISTS CPACK_APK_NO_COMPRESS_EXTENSIONS)
  list(APPEND _link_options -0 "${_extension}")
endforeach()
# Of the asset directories given to aapt2 link, the first one holding a file
# wins, so the later directories, which override the earlier ones, go first.
set(_asset_directories ${CPACK_APK_ASSET_DIRECTORIES})
list(REVERSE _asset_directories)
foreach(_asset_directory IN LISTS _asset_directories)
  if(NOT IS_DIRECTORY "${_asset_directory}")
    message(FATAL_ERROR
      "CPack APK: '${_asset_directory}' listed in "
      "CPACK_APK_ASSET_DIRECTORIES is not a directory.")
  endif()
  list(APPEND _link_options -A "${_asset_directory}")
endforeach()
# These only apply when the manifest leaves the versions out.
if("${CPACK_APK_VERSION_CODE}" STREQUAL "")
  list(APPEND _link_options --version-code 1)
endif()
if("${CPACK_APK_VERSION_NAME}" STREQUAL "" AND CPACK_PACKAGE_VERSION)
  list(APPEND _link_options --version-name "${CPACK_PACKAGE_VERSION}")
endif()

# Generate R.java so that the application sources can refer to the resources.
# The code of an Android archive refers to them through an R class of its own
# package, which the archive does not ship, so generate those as well.
set(_generated_java_directory "${CPACK_APK_WORK_DIRECTORY}/gen")
if(_compiled_resources AND (_java_sources OR _aar_packages))
  file(MAKE_DIRECTORY "${_generated_java_directory}")
  list(APPEND _link_options --java "${_generated_java_directory}")
  if(_aar_packages)
    list(JOIN _aar_packages ":" _extra_packages)
    list(APPEND _link_options --extra-packages "${_extra_packages}")
  endif()
endif()

# Without a target API level, Android takes the minimum one, and recent
# versions refuse to install applications targeting old API levels.  Like
# Gradle, default it to the compile API level.  `aapt2` only uses this for
# manifests leaving it out.
if(CPACK_APK_COMPILE_SDK_VERSION MATCHES "^([0-9]+)")
  list(APPEND _link_options --target-sdk-version "${CMAKE_MATCH_1}")
endif()

set(_apk "${CPACK_APK_WORK_DIRECTORY}/unaligned.apk")
_cpack_apk_run("aapt2 link"
  "${CPACK_APK_AAPT2_EXECUTABLE}" link
  -o "${_apk}"
  -I "${CPACK_APK_ANDROID_JAR}"
  --manifest "${CPACK_APK_MANIFEST}"
  --auto-add-overlay
  ${_link_options}
  ${CPACK_APK_AAPT2_LINK_EXTRA_OPTIONS}
  ${_compiled_resources}
  )

#-----------------------------------------------------------------------------
# Compile the Java sources and convert the bytecode to Dalvik executables
#-----------------------------------------------------------------------------

if(CMAKE_HOST_WIN32)
  set(_class_path_separator ";")
else()
  set(_class_path_separator ":")
endif()

# `javac` checks the Java library calls against the JDK it belongs to, which
# is not the library Android provides.  Build a system image out of the one
# of the platform instead, the way the Android Gradle plugin does, so that
# the calls are checked against what is available on a device.
function(_cpack_apk_java_system_image result)
  set(${result} "" PARENT_SCOPE)
  get_filename_component(_platform_directory "${CPACK_APK_ANDROID_JAR}"
    DIRECTORY)
  set(_core_jar "${_platform_directory}/core-for-system-modules.jar")
  # `javac` reads the image through the file system provider of the JDK.
  set(_jrt_fs_jar "${CPACK_APK_JAVA_HOME}/lib/jrt-fs.jar")
  if(NOT EXISTS "${_core_jar}" OR NOT EXISTS "${_jrt_fs_jar}"
      OR NOT Java_VERSION_MAJOR)
    _cpack_apk_debug("No Java system image for the platform, compiling "
      "against the Java library of the JDK.")
    return()
  endif()
  foreach(_java_tool IN ITEMS jmod jlink)
    string(TOUPPER "${_java_tool}" _java_tool_var)
    if(NOT CPACK_APK_${_java_tool_var}_EXECUTABLE)
      find_program(CPACK_APK_${_java_tool_var}_EXECUTABLE
        NAMES ${_java_tool}
        HINTS "${CPACK_APK_JAVA_HOME}/bin"
        NO_DEFAULT_PATH
        )
    endif()
    if(NOT CPACK_APK_${_java_tool_var}_EXECUTABLE)
      _cpack_apk_debug("No '${_java_tool}', compiling against the Java "
        "library of the JDK.")
      return()
    endif()
  endforeach()

  set(_system_directory "${CPACK_APK_WORK_DIRECTORY}/java-system")
  set(_classes "${_system_directory}/classes")
  file(ARCHIVE_EXTRACT INPUT "${_core_jar}" DESTINATION "${_classes}")

  # The image holds a single `java.base` module exporting every package.
  set(_module_info "module java.base {\n")
  file(GLOB_RECURSE _entries LIST_DIRECTORIES TRUE
    RELATIVE "${_classes}" "${_classes}/*")
  foreach(_entry IN LISTS _entries)
    if(IS_DIRECTORY "${_classes}/${_entry}")
      file(GLOB _package_classes "${_classes}/${_entry}/*.class")
      if(_package_classes)
        string(REPLACE "/" "." _package "${_entry}")
        string(APPEND _module_info "  exports ${_package};\n")
      endif()
    endif()
  endforeach()
  string(APPEND _module_info "}\n")
  file(WRITE "${_system_directory}/module-info.java" "${_module_info}")
  _cpack_apk_run("javac of the Java system module"
    ${CPACK_APK_ENV} "${Java_JAVAC_EXECUTABLE}"
    --system none --patch-module "java.base=${_classes}"
    -d "${_classes}" "${_system_directory}/module-info.java"
    )

  # `jlink` insists on the module version matching its own, and on a target
  # platform being recorded, although nothing platform specific is in there.
  file(MAKE_DIRECTORY "${_system_directory}/jmods")
  _cpack_apk_run("jmod of the Java system module"
    ${CPACK_APK_ENV} "${CPACK_APK_JMOD_EXECUTABLE}" create
    --module-version "${Java_VERSION_MAJOR}"
    --target-platform linux-amd64
    --class-path "${_classes}"
    "${_system_directory}/jmods/java.base.jmod"
    )
  set(_image "${_system_directory}/image")
  _cpack_apk_run("jlink of the Java system image"
    ${CPACK_APK_ENV} "${CPACK_APK_JLINK_EXECUTABLE}"
    --module-path "${_system_directory}/jmods"
    --add-modules java.base
    --disable-plugin system-modules
    --output "${_image}"
    )
  file(COPY "${_jrt_fs_jar}" DESTINATION "${_image}/lib")
  set(${result} "${_image}" PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE _generated_java_sources "${_generated_java_directory}/*.java")
list(APPEND _java_sources ${_generated_java_sources})

if(_java_sources)
  if(NOT Java_JAVAC_EXECUTABLE)
    message(FATAL_ERROR
      "CPack APK: 'javac' not found, but there are Java sources to compile.  "
      "A JDK is required.")
  endif()

  set(_classes_directory "${CPACK_APK_WORK_DIRECTORY}/classes")
  file(MAKE_DIRECTORY "${_classes_directory}")

  set(_class_path "${CPACK_APK_ANDROID_JAR}")
  foreach(_jar IN LISTS CPACK_APK_JARS)
    string(APPEND _class_path "${_class_path_separator}${_jar}")
  endforeach()

  # Write the classpath and the sources to an argument file: the source list
  # easily grows past the command line length limit, and on Windows the
  # classpath separator is the very character a CMake list is split on, so a
  # classpath does not survive being passed along as a single argument.
  # Arguments in an argument file are separated by whitespace, hence the
  # quoting, and a backslash between the quotes is an escape character, hence
  # the forward slashes `javac` accepts on every platform.
  set(_javac_argument_file "${CPACK_APK_WORK_DIRECTORY}/javac-arguments.txt")
  string(REPLACE "\\" "/" _class_path "${_class_path}")
  set(_javac_arguments "-classpath \"${_class_path}\"\n")

  # A system image cannot be combined with `--release`, nor used for a
  # target release older than 9, where `android.jar` becomes the boot class
  # path instead.
  set(_javac_release)
  set(_javac_source)
  set(_javac_target)
  set(_javac_options ${CPACK_APK_JAVAC_EXTRA_OPTIONS})
  while(_javac_options)
    list(POP_FRONT _javac_options _javac_option)
    if(_javac_option MATCHES "^--?(release|source|target)=(.*)$")
      set(_javac_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
    elseif(_javac_option MATCHES "^(--release|--?source|--?target)$"
        AND _javac_options)
      string(REGEX REPLACE "^-+" "" _javac_option "${_javac_option}")
      list(POP_FRONT _javac_options _javac_${_javac_option})
    endif()
  endwhile()
  # Left to itself, `javac` targets the Java release of the JDK, whose class
  # files the `d8` of older build tools cannot read.
  if(NOT _javac_release AND NOT _javac_source AND NOT _javac_target)
    if(NOT DEFINED CPACK_APK_JAVA_VERSION)
      set(CPACK_APK_JAVA_VERSION 11)
    endif()
    if(NOT "${CPACK_APK_JAVA_VERSION}" STREQUAL "")
      set(_javac_source "${CPACK_APK_JAVA_VERSION}")
      set(_javac_target "${CPACK_APK_JAVA_VERSION}")
      string(APPEND _javac_arguments
        "-source ${_javac_source}\n-target ${_javac_target}\n")
    endif()
  endif()
  if(NOT _javac_target)
    set(_javac_target "${_javac_source}")
  endif()
  if(_javac_release)
    _cpack_apk_debug("javac targets release ${_javac_release}, compiling "
      "against the Java library of the JDK.")
  elseif(_javac_target MATCHES "^(1\\.)?[0-8]$")
    string(REPLACE "\\" "/" _boot_class_path "${CPACK_APK_ANDROID_JAR}")
    string(APPEND _javac_arguments "-bootclasspath \"${_boot_class_path}\"\n")
  else()
    _cpack_apk_java_system_image(_java_system_image)
    if(_java_system_image)
      string(REPLACE "\\" "/" _java_system_image "${_java_system_image}")
      string(APPEND _javac_arguments "--system \"${_java_system_image}\"\n")
    endif()
  endif()
  foreach(_java_source IN LISTS _java_sources)
    string(REPLACE "\\" "/" _java_source "${_java_source}")
    string(APPEND _javac_arguments "\"${_java_source}\"\n")
  endforeach()
  file(WRITE "${_javac_argument_file}" "${_javac_arguments}")

  _cpack_apk_run("javac"
    ${CPACK_APK_ENV} "${Java_JAVAC_EXECUTABLE}"
    -d "${_classes_directory}"
    ${CPACK_APK_JAVAC_EXTRA_OPTIONS}
    "@${_javac_argument_file}"
    )

  set(_classes_jar "${CPACK_APK_WORK_DIRECTORY}/classes.jar")
  _cpack_apk_run("jar of the compiled Java sources"
    ${CPACK_APK_ENV} "${CPACK_APK_JAR_EXECUTABLE}"
    cfM "${_classes_jar}" -C "${_classes_directory}" .
    )
  list(APPEND CPACK_APK_JARS "${_classes_jar}")
endif()

set(_dex_directory "${CPACK_APK_WORK_DIRECTORY}/dex")
if(CPACK_APK_JARS)
  file(MAKE_DIRECTORY "${_dex_directory}")
  set(_d8_options)
  if(_min_sdk_version)
    list(APPEND _d8_options --min-api "${_min_sdk_version}")
  endif()
  # `d8` compiles with debugging information unless told otherwise, and
  # rejects being told both.
  if(NOT "--debug" IN_LIST CPACK_APK_D8_EXTRA_OPTIONS
      AND NOT "--release" IN_LIST CPACK_APK_D8_EXTRA_OPTIONS)
    list(APPEND _d8_options --release)
  endif()
  # On Windows, `d8` is a batch script, and its command line is subject to
  # the length limit of `cmd.exe`.  An argument file of `d8` holds one
  # argument per line, taken as is.
  set(_d8_argument_file "${CPACK_APK_WORK_DIRECTORY}/d8-arguments.txt")
  list(JOIN CPACK_APK_JARS "\n" _d8_arguments)
  file(WRITE "${_d8_argument_file}" "${_d8_arguments}\n")
  _cpack_apk_run("d8"
    ${CPACK_APK_ENV} "${CPACK_APK_D8_EXECUTABLE}"
    --lib "${CPACK_APK_ANDROID_JAR}"
    --output "${_dex_directory}"
    ${_d8_options}
    ${CPACK_APK_D8_EXTRA_OPTIONS}
    "@${_d8_argument_file}"
    )
endif()

#-----------------------------------------------------------------------------
# Lay out everything else that goes into the APK
#-----------------------------------------------------------------------------

# Entries added with `jar -0` are stored uncompressed so that `zipalign` can
# page-align them; the rest are deflated.
set(_deflated_directory "${CPACK_APK_WORK_DIRECTORY}/deflated")
set(_stored_directory "${CPACK_APK_WORK_DIRECTORY}/stored")
file(MAKE_DIRECTORY "${_deflated_directory}" "${_stored_directory}")

file(GLOB _dex_files RELATIVE "${_dex_directory}" "${_dex_directory}/*.dex")
foreach(_dex_file IN LISTS _dex_files)
  file(COPY "${_dex_directory}/${_dex_file}"
    DESTINATION "${_deflated_directory}")
endforeach()

file(GLOB_RECURSE _staged_files
  RELATIVE "${CPACK_APK_STAGING_DIRECTORY}" "${CPACK_APK_STAGING_DIRECTORY}/*")

# Copy the native library `source` to `destination`, stripped of its debug
# symbols the way the Android Gradle plugin does, unless its name matches
# CPACK_APK_KEEP_DEBUG_SYMBOLS.  Files that are not ELF, such as those of a
# host build, are copied as they are.  Unlike file(COPY), which skips files
# whose time stamps match, as those of libraries built reproducibly may,
# this replaces a library of an archive.
function(_cpack_apk_copy_native_library source destination)
  get_filename_component(_destination_directory "${destination}" DIRECTORY)
  file(MAKE_DIRECTORY "${_destination_directory}")
  get_filename_component(_name "${destination}" NAME)
  set(_strip TRUE)
  foreach(_regex IN LISTS _keep_debug_symbols_regexes)
    if(_name MATCHES "${_regex}")
      set(_strip FALSE)
      break()
    endif()
  endforeach()
  file(READ "${source}" _magic LIMIT 4 HEX)
  if(NOT _magic STREQUAL "7f454c46")
    set(_strip FALSE)
  elseif(NOT _name STREQUAL "libc++_shared.so" AND CPACK_READELF_EXECUTABLE)
    # Note the ABI of a library linked to the shared C++ runtime.
    set(_command ${CPACK_READELF_EXECUTABLE} -d "${source}")
    list(JOIN _command " " _command_text)
    _cpack_apk_debug("Running readelf: ${_command_text}")
    execute_process(
      COMMAND ${_command}
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE _output
      ERROR_VARIABLE _error
      )
    if(NOT _result EQUAL 0)
      message(WARNING
        "CPack APK: Unable to read the dynamic section of '${source}'.\n"
        "Command: ${_command_text}\n${_error}")
    elseif(_output MATCHES "\\(NEEDED\\)[^\n]*\\[libc\\+\\+_shared\\.so\\]")
      get_filename_component(_abi "${_destination_directory}" NAME)
      set_property(GLOBAL APPEND PROPERTY _cpack_apk_cxx_shared_abis "${_abi}")
    endif()
  elseif(NOT _name STREQUAL "libc++_shared.so")
    set_property(GLOBAL APPEND PROPERTY _cpack_apk_unchecked "${source}")
  endif()
  if(_strip AND CPACK_APK_STRIP_EXECUTABLE)
    set(_command ${CPACK_APK_STRIP_EXECUTABLE}
      --strip-unneeded -o "${destination}" "${source}")
    list(JOIN _command " " _command_text)
    _cpack_apk_debug("Running strip: ${_command_text}")
    execute_process(
      COMMAND ${_command}
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE _output
      ERROR_VARIABLE _output
      )
    if(_result EQUAL 0)
      return()
    endif()
    message(WARNING
      "CPack APK: Unable to strip '${source}', packaging it as it is.\n"
      "Command: ${_command_text}\n${_output}")
  elseif(_strip)
    set_property(GLOBAL APPEND PROPERTY _cpack_apk_unstripped "${source}")
  endif()
  file(COPY_FILE "${source}" "${destination}")
endfunction()
set_property(GLOBAL PROPERTY _cpack_apk_unstripped)
set_property(GLOBAL PROPERTY _cpack_apk_cxx_shared_abis)
set_property(GLOBAL PROPERTY _cpack_apk_unchecked)

# Native libraries of the Android archives go to `lib/<abi>/`.  Android
# picks the ABI to load libraries for by what is in the APK, so only the ABIs
# the application itself ships code for are packaged.  The application's own
# libraries, copied after these, take precedence.
set(_abis)
if(CPACK_APK_ABI)
  list(APPEND _abis "${CPACK_APK_ABI}")
endif()
foreach(_staged_file IN LISTS _staged_files)
  if(_staged_file MATCHES "^lib/([^/]+)/[^/]+\\.so$")
    list(APPEND _abis "${CMAKE_MATCH_1}")
  endif()
endforeach()
list(REMOVE_DUPLICATES _abis)
foreach(_aar_library IN LISTS _aar_native_libraries)
  get_filename_component(_abi_directory "${_aar_library}" DIRECTORY)
  get_filename_component(_abi "${_abi_directory}" NAME)
  if(_abis AND NOT _abi IN_LIST _abis)
    continue()
  endif()
  if(".so" IN_LIST CPACK_APK_NO_COMPRESS_EXTENSIONS)
    set(_destination "${_stored_directory}")
  else()
    set(_destination "${_deflated_directory}")
  endif()
  get_filename_component(_aar_library_name "${_aar_library}" NAME)
  _cpack_apk_copy_native_library("${_aar_library}"
    "${_destination}/lib/${_abi}/${_aar_library_name}")
endforeach()

foreach(_staged_file IN LISTS _staged_files)
  _cpack_apk_is_consumed("${_staged_file}" _skip)
  if(_skip)
    continue()
  endif()

  # A shared library installed straight into `lib/` carries no ABI
  # sub-directory, which Android requires.
  set(_entry "${_staged_file}")
  if(CPACK_APK_ABI AND _entry MATCHES "^lib/[^/]+\\.so$")
    string(REPLACE "lib/" "lib/${CPACK_APK_ABI}/" _entry "${_entry}")
  endif()

  get_filename_component(_extension "${_staged_file}" LAST_EXT)
  if(_extension IN_LIST CPACK_APK_NO_COMPRESS_EXTENSIONS)
    set(_destination "${_stored_directory}")
  else()
    set(_destination "${_deflated_directory}")
  endif()

  if(_entry MATCHES "^lib/[^/]+/[^/]+\\.so$")
    _cpack_apk_copy_native_library(
      "${CPACK_APK_STAGING_DIRECTORY}/${_staged_file}"
      "${_destination}/${_entry}")
    continue()
  endif()
  # Unlike file(COPY), which skips files whose time stamps match, as those of
  # libraries built reproducibly may, replace a library of an archive.
  get_filename_component(_entry_directory "${_destination}/${_entry}"
    DIRECTORY)
  file(MAKE_DIRECTORY "${_entry_directory}")
  file(COPY_FILE "${CPACK_APK_STAGING_DIRECTORY}/${_staged_file}"
    "${_destination}/${_entry}")
endforeach()

# Like the Android Gradle plugin, package the shared C++ runtime of the NDK
# for the ABIs whose libraries are linked to it, unless it is packaged
# already.
get_property(_cxx_shared_abis GLOBAL PROPERTY _cpack_apk_cxx_shared_abis)
list(REMOVE_DUPLICATES _cxx_shared_abis)
set(_cxx_shared_missing)
foreach(_abi IN LISTS _cxx_shared_abis)
  if(EXISTS "${_stored_directory}/lib/${_abi}/libc++_shared.so" OR
      EXISTS "${_deflated_directory}/lib/${_abi}/libc++_shared.so")
    continue()
  endif()
  set(_triple)
  if(_abi STREQUAL "arm64-v8a")
    set(_triple aarch64-linux-android)
  elseif(_abi STREQUAL "armeabi-v7a")
    set(_triple arm-linux-androideabi)
  elseif(_abi STREQUAL "x86")
    set(_triple i686-linux-android)
  elseif(_abi STREQUAL "x86_64")
    set(_triple x86_64-linux-android)
  elseif(_abi STREQUAL "riscv64")
    set(_triple riscv64-linux-android)
  endif()
  set(_cxx_shared "${_sysroot_libraries}/${_triple}/libc++_shared.so")
  if(NOT _sysroot_libraries OR NOT _triple OR NOT EXISTS "${_cxx_shared}")
    list(APPEND _cxx_shared_missing "${_abi}")
    continue()
  endif()
  if(".so" IN_LIST CPACK_APK_NO_COMPRESS_EXTENSIONS)
    set(_destination "${_stored_directory}")
  else()
    set(_destination "${_deflated_directory}")
  endif()
  _cpack_apk_copy_native_library("${_cxx_shared}"
    "${_destination}/lib/${_abi}/libc++_shared.so")
endforeach()
if(_cxx_shared_missing)
  list(JOIN _cxx_shared_missing ", " _cxx_shared_missing)
  message(WARNING
    "CPack APK: Native libraries for ${_cxx_shared_missing} are linked to "
    "libc++_shared.so, but no Android NDK providing it was found.  Set "
    "CPACK_APK_NDK_ROOT, or install libc++_shared.so into lib/<abi>/.")
endif()
get_property(_unchecked GLOBAL PROPERTY _cpack_apk_unchecked)
if(_unchecked)
  list(JOIN _unchecked "\n  " _unchecked)
  message(WARNING
    "CPack APK: No readelf found, unable to tell whether these native "
    "libraries are linked to the shared C++ runtime, libc++_shared.so:\n"
    "  ${_unchecked}\n"
    "Install the NDK with the Android SDK manager, or set "
    "CPACK_APK_NDK_ROOT or CPACK_READELF_EXECUTABLE.")
endif()

# Like the Android Gradle plugin, package the libraries as they are when
# there is nothing to strip them with.
get_property(_unstripped GLOBAL PROPERTY _cpack_apk_unstripped)
if(_unstripped)
  list(JOIN _unstripped "\n  " _unstripped)
  message(WARNING
    "CPack APK: No llvm-strip found in the Android NDK, packaging these "
    "native libraries without stripping their debug symbols:\n"
    "  ${_unstripped}\n"
    "Install the NDK with the Android SDK manager, or set "
    "CPACK_APK_NDK_ROOT or CPACK_APK_STRIP_EXECUTABLE.")
endif()

foreach(_mode IN ITEMS deflated stored)
  if(_mode STREQUAL "stored")
    set(_jar_options u0fM)
    set(_directory "${_stored_directory}")
  else()
    set(_jar_options ufM)
    set(_directory "${_deflated_directory}")
  endif()
  file(GLOB _entries "${_directory}/*")
  if(NOT _entries)
    continue()
  endif()
  _cpack_apk_run("jar update of the ${_mode} APK entries"
    ${CPACK_APK_ENV} "${CPACK_APK_JAR_EXECUTABLE}"
    ${_jar_options} "${_apk}" -C "${_directory}" .
    )
endforeach()

#-----------------------------------------------------------------------------
# Align and sign
#-----------------------------------------------------------------------------

set(_aligned_apk "${CPACK_APK_WORK_DIRECTORY}/aligned.apk")
if(NOT DEFINED CPACK_APK_ZIPALIGN_OPTIONS)
  # Devices with 16 KB memory pages can only map uncompressed native
  # libraries that are aligned to them.  Older build tools only know 4 KB.
  if(CPACK_APK_BUILD_TOOLS_VERSION VERSION_LESS 35)
    set(CPACK_APK_ZIPALIGN_OPTIONS -p -f 4)
  else()
    set(CPACK_APK_ZIPALIGN_OPTIONS -P 16 -f 4)
  endif()
endif()
_cpack_apk_run("zipalign"
  ${CPACK_APK_ENV} "${CPACK_APK_ZIPALIGN_EXECUTABLE}"
  ${CPACK_APK_ZIPALIGN_OPTIONS} "${_apk}" "${_aligned_apk}"
  )

if(NOT DEFINED CPACK_APK_SIGN)
  set(CPACK_APK_SIGN TRUE)
endif()

if(NOT CPACK_APK_SIGN)
  file(RENAME "${_aligned_apk}" "${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
  message(WARNING
    "CPack APK: '${CPACK_TEMPORARY_PACKAGE_FILE_NAME}' is not signed and "
    "cannot be installed on a device as-is.")
  return()
endif()

# The passwords reach the signing tools through the environment, which the
# tools inherit from this process.  On the command line, other users could
# read them off the process list.
function(_cpack_apk_export_passwords)
  set(ENV{CPACK_APK_KEYSTORE_PASSWORD} "${CPACK_APK_KEYSTORE_PASSWORD}")
  set(ENV{CPACK_APK_KEY_PASSWORD} "${CPACK_APK_KEY_PASSWORD}")
endfunction()

if(NOT CPACK_APK_KEYSTORE)
  # Match the debug keystore the Android tooling uses, so that incremental
  # installs onto a device keep working.
  # That is `ANDROID_USER_HOME`, or else the `.android` directory of the
  # user's home, as the JVM knows it: on Windows, that is the profile
  # directory, whatever `HOME` may say.
  set(_android_user_home)
  if(NOT "$ENV{ANDROID_USER_HOME}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{ANDROID_USER_HOME}" _android_user_home)
  else()
    if(CMAKE_HOST_WIN32)
      set(_home_vars ANDROID_SDK_HOME USERPROFILE HOME)
    else()
      set(_home_vars ANDROID_SDK_HOME HOME)
    endif()
    foreach(_var IN LISTS _home_vars)
      if(DEFINED ENV{${_var}} AND IS_DIRECTORY "$ENV{${_var}}")
        file(TO_CMAKE_PATH "$ENV{${_var}}/.android" _android_user_home)
        break()
      endif()
    endforeach()
  endif()
  if(NOT _android_user_home)
    message(FATAL_ERROR
      "CPack APK: Cannot determine the home directory to hold the default "
      "debug keystore.  Set CPACK_APK_KEYSTORE to a keystore of your own.")
  endif()
  set(CPACK_APK_KEYSTORE "${_android_user_home}/debug.keystore")
  set(CPACK_APK_KEYSTORE_PASSWORD "android")
  set(CPACK_APK_KEY_ALIAS "androiddebugkey")
  set(CPACK_APK_KEY_PASSWORD "android")

  if(NOT EXISTS "${CPACK_APK_KEYSTORE}")
    message(STATUS
      "CPack APK: Generating a debug keystore at '${CPACK_APK_KEYSTORE}'.")
    get_filename_component(_keystore_directory
      "${CPACK_APK_KEYSTORE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_keystore_directory}")
    _cpack_apk_export_passwords()
    _cpack_apk_run("keytool"
      ${CPACK_APK_ENV} "${CPACK_APK_KEYTOOL_EXECUTABLE}"
      -genkeypair
      -keystore "${CPACK_APK_KEYSTORE}"
      -storepass:env CPACK_APK_KEYSTORE_PASSWORD
      -alias "${CPACK_APK_KEY_ALIAS}"
      -keypass:env CPACK_APK_KEY_PASSWORD
      -keyalg RSA -keysize 2048 -validity 10000
      -dname "CN=Android Debug,O=Android,C=US"
      )
  endif()
elseif(NOT EXISTS "${CPACK_APK_KEYSTORE}")
  message(FATAL_ERROR
    "CPack APK: CPACK_APK_KEYSTORE is set to '${CPACK_APK_KEYSTORE}', which "
    "does not exist.")
endif()

if(NOT CPACK_APK_KEY_ALIAS)
  message(FATAL_ERROR
    "CPack APK: CPACK_APK_KEY_ALIAS must name the key to sign with inside "
    "'${CPACK_APK_KEYSTORE}'.")
endif()
if(NOT DEFINED CPACK_APK_KEY_PASSWORD)
  set(CPACK_APK_KEY_PASSWORD "${CPACK_APK_KEYSTORE_PASSWORD}")
endif()

# `apksigner` reads the minimum API level from the manifest of the APK.
_cpack_apk_export_passwords()
_cpack_apk_run("apksigner"
  ${CPACK_APK_ENV} "${CPACK_APK_APKSIGNER_EXECUTABLE}" sign
  --ks "${CPACK_APK_KEYSTORE}"
  --ks-pass env:CPACK_APK_KEYSTORE_PASSWORD
  --ks-key-alias "${CPACK_APK_KEY_ALIAS}"
  --key-pass env:CPACK_APK_KEY_PASSWORD
  ${CPACK_APK_APKSIGNER_EXTRA_OPTIONS}
  --out "${CPACK_TEMPORARY_PACKAGE_FILE_NAME}"
  "${_aligned_apk}"
  )
unset(ENV{CPACK_APK_KEYSTORE_PASSWORD})
unset(ENV{CPACK_APK_KEY_PASSWORD})

_cpack_apk_run("apksigner verify"
  ${CPACK_APK_ENV} "${CPACK_APK_APKSIGNER_EXECUTABLE}" verify
  "${CPACK_TEMPORARY_PACKAGE_FILE_NAME}"
  )
