# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.13...3.25 FATAL_ERROR)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_C ${CMAKE_CURRENT_SOURCE_DIR}/Source/Modules/OverrideC.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/Source/Modules/OverrideCXX.cmake)

project(CMake)
unset(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX)
unset(CMAKE_USER_MAKE_RULES_OVERRIDE_C)

# FIXME: This block should go away after a transition period.
if(MSVC AND NOT CMAKE_VERSION VERSION_LESS 3.15)
  # Filter out MSVC runtime library flags that may have come from
  # the cache of an existing build tree or from scripts.
  foreach(l IN ITEMS C CXX)
    foreach(c IN ITEMS DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)
      string(REGEX REPLACE "[-/]M[DT]d?( |$)" "" "CMAKE_${l}_FLAGS_${c}" "${CMAKE_${l}_FLAGS_${c}}")
    endforeach()
  endforeach()
endif()

# Make sure we can find internal find_package modules only used for
# building CMake and not for shipping externally
list(INSERT CMAKE_MODULE_PATH 0 ${CMake_SOURCE_DIR}/Source/Modules)

if(CMAKE_BOOTSTRAP)
  # Running from bootstrap script.  Set local variable and remove from cache.
  set(CMAKE_BOOTSTRAP 1)
  unset(CMAKE_BOOTSTRAP CACHE)
endif()

if(CMake_TEST_HOST_CMAKE)
  get_filename_component(CMake_TEST_EXTERNAL_CMAKE "${CMAKE_COMMAND}" DIRECTORY)
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
    message(FATAL_ERROR
      "CMake no longer compiles on HP-UX.  See\n"
      "  https://gitlab.kitware.com/cmake/cmake/-/issues/17137\n"
      "Use CMake 3.9 or lower instead."
      )
  endif()

  set(CMake_BIN_DIR ${CMake_BINARY_DIR}/bin)
endif()

if(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL)
  if(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL MATCHES "^3|2\\.1$")
    set(USE_LGPL "${CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL}")
  else()
    set(USE_LGPL "2.1")
  endif()
else()
  set(USE_LGPL "")
endif()

# Use most-recent available language dialects with GNU and Clang
if(NOT DEFINED CMAKE_C_STANDARD AND NOT CMake_NO_C_STANDARD)
  include(${CMake_SOURCE_DIR}/Source/Checks/cm_c11_thread_local.cmake)
  if(NOT CMake_C11_THREAD_LOCAL_BROKEN)
    set(CMAKE_C_STANDARD 11)
  else()
    set(CMAKE_C_STANDARD 99)
  endif()
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD AND NOT CMake_NO_CXX_STANDARD)
  if(CMAKE_CXX_COMPILER_ID STREQUAL SunPro AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.14)
    set(CMAKE_CXX_STANDARD 98)
  else()
    include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx17_check.cmake)
    if(NOT CMake_CXX17_BROKEN)
      set(CMAKE_CXX_STANDARD 17)
    else()
      include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx14_check.cmake)
      if(NOT CMake_CXX14_BROKEN)
        set(CMAKE_CXX_STANDARD 14)
      else()
        set(CMAKE_CXX_STANDARD 11)
      endif()
    endif()
  endif()
endif()
if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # include special compile flags for some compilers
  include(CompileFlags.cmake)

  # check for available C++ features
  include(${CMake_SOURCE_DIR}/Source/Checks/cm_cxx_features.cmake)

  if(NOT CMake_HAVE_CXX_UNIQUE_PTR)
    message(FATAL_ERROR "The C++ compiler does not support C++11 (e.g. std::unique_ptr).")
  endif()
endif()

# Inform STL library header wrappers whether to use system versions.
configure_file(Utilities/std/cmSTL.hxx.in Utilities/cmSTL.hxx @ONLY)

# set the internal encoding of CMake to UTF-8
set(KWSYS_ENCODING_DEFAULT_CODEPAGE CP_UTF8)

# option to use COMPONENT with install command
option(CMake_INSTALL_COMPONENTS "Using components when installing" OFF)
mark_as_advanced(CMake_INSTALL_COMPONENTS)
macro(CMake_OPTIONAL_COMPONENT NAME)
  if(CMake_INSTALL_COMPONENTS)
    set(COMPONENT COMPONENT ${NAME})
  else()
    set(COMPONENT)
  endif()
endmacro()

# option to disable installing 3rd-party dependencies
option(CMake_INSTALL_DEPENDENCIES
  "Whether to install 3rd-party runtime dependencies" OFF)
mark_as_advanced(CMake_INSTALL_DEPENDENCIES)

# option to build reference for CMake developers
option(CMake_BUILD_DEVELOPER_REFERENCE
  "Build CMake Developer Reference" OFF)
mark_as_advanced(CMake_BUILD_DEVELOPER_REFERENCE)

# option to build using interprocedural optimizations (IPO/LTO)
option(CMake_BUILD_LTO "Compile CMake with link-time optimization if supported" OFF)
if(CMake_BUILD_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT HAVE_IPO)
  if(HAVE_IPO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

# Check whether to build support for the debugger mode.
if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(NOT DEFINED CMake_ENABLE_DEBUGGER)
    # The debugger uses cppdap, which does not compile everywhere.
    if(CMAKE_SYSTEM_NAME MATCHES "Windows|Darwin|Linux|BSD|DragonFly|CYGWIN|MSYS"
        AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.16)
        AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "XLClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.1)
        )
      set(CMake_ENABLE_DEBUGGER 1)
    else()
      set(CMake_ENABLE_DEBUGGER 0)
    endif()
  endif()
else()
  set(CMake_ENABLE_DEBUGGER 0)
endif()

#-----------------------------------------------------------------------
# a macro to deal with system libraries, implemented as a macro
# simply to improve readability of the main script
#-----------------------------------------------------------------------
macro(CMAKE_HANDLE_SYSTEM_LIBRARIES)
  # Options have dependencies.
  include(CMakeDependentOption)

  # Allow the user to enable/disable all system utility library options by
  # defining CMAKE_USE_SYSTEM_LIBRARIES or CMAKE_USE_SYSTEM_LIBRARY_${util}.
  set(UTILITIES BZIP2 CPPDAP CURL EXPAT FORM JSONCPP LIBARCHIVE LIBLZMA LIBRHASH LIBUV NGHTTP2 ZLIB ZSTD)
  foreach(util IN LISTS UTILITIES)
    if(NOT DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util}
        AND DEFINED CMAKE_USE_SYSTEM_LIBRARIES)
      set(CMAKE_USE_SYSTEM_LIBRARY_${util} "${CMAKE_USE_SYSTEM_LIBRARIES}")
    endif()
    if(DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util})
      if(CMAKE_USE_SYSTEM_LIBRARY_${util})
        set(CMAKE_USE_SYSTEM_LIBRARY_${util} ON)
      else()
        set(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
      endif()
      if(CMAKE_BOOTSTRAP)
        unset(CMAKE_USE_SYSTEM_LIBRARY_${util} CACHE)
      endif()
      string(TOLOWER "${util}" lutil)
      set(CMAKE_USE_SYSTEM_${util} "${CMAKE_USE_SYSTEM_LIBRARY_${util}}"
        CACHE BOOL "Use system-installed ${lutil}" FORCE)
    else()
      set(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
    endif()
  endforeach()
  if(CMAKE_BOOTSTRAP)
    unset(CMAKE_USE_SYSTEM_LIBRARIES CACHE)
  endif()

  # Optionally use system utility libraries.
  option(CMAKE_USE_SYSTEM_LIBARCHIVE "Use system-installed libarchive" "${CMAKE_USE_SYSTEM_LIBRARY_LIBARCHIVE}")
  if(CMake_ENABLE_DEBUGGER)
    option(CMAKE_USE_SYSTEM_CPPDAP "Use system-installed cppdap" "${CMAKE_USE_SYSTEM_LIBRARY_CPPDAP}")
  endif()
  option(CMAKE_USE_SYSTEM_CURL "Use system-installed curl" "${CMAKE_USE_SYSTEM_LIBRARY_CURL}")
  option(CMAKE_USE_SYSTEM_EXPAT "Use system-installed expat" "${CMAKE_USE_SYSTEM_LIBRARY_EXPAT}")
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZLIB "Use system-installed zlib"
    "${CMAKE_USE_SYSTEM_LIBRARY_ZLIB}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE;NOT CMAKE_USE_SYSTEM_CURL" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_BZIP2 "Use system-installed bzip2"
    "${CMAKE_USE_SYSTEM_LIBRARY_BZIP2}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZSTD "Use system-installed zstd"
    "${CMAKE_USE_SYSTEM_LIBRARY_ZSTD}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_LIBLZMA "Use system-installed liblzma"
    "${CMAKE_USE_SYSTEM_LIBRARY_LIBLZMA}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_NGHTTP2 "Use system-installed nghttp2"
    "${CMAKE_USE_SYSTEM_LIBRARY_NGHTTP2}" "NOT CMAKE_USE_SYSTEM_CURL" ON)
  option(CMAKE_USE_SYSTEM_FORM "Use system-installed libform" "${CMAKE_USE_SYSTEM_LIBRARY_FORM}")
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_JSONCPP "Use system-installed jsoncpp"
    "${CMAKE_USE_SYSTEM_LIBRARY_JSONCPP}" "NOT CMAKE_USE_SYSTEM_CPPDAP" ON)
  option(CMAKE_USE_SYSTEM_LIBRHASH "Use system-installed librhash" "${CMAKE_USE_SYSTEM_LIBRARY_LIBRHASH}")
  option(CMAKE_USE_SYSTEM_LIBUV "Use system-installed libuv" "${CMAKE_USE_SYSTEM_LIBRARY_LIBUV}")

  # For now use system KWIML only if explicitly requested rather
  # than activating via the general system libs options.
  option(CMAKE_USE_SYSTEM_KWIML "Use system-installed KWIML" OFF)
  mark_as_advanced(CMAKE_USE_SYSTEM_KWIML)

  # Mention to the user what system libraries are being used.
  foreach(util IN LISTS UTILITIES ITEMS KWIML)
    if(CMAKE_USE_SYSTEM_${util})
      message(STATUS "Using system-installed ${util}")
    endif()
  endforeach()

  # Inform utility library header wrappers whether to use system versions.
  configure_file(Utilities/cmThirdParty.h.in Utilities/cmThirdParty.h @ONLY)

endmacro()

#-----------------------------------------------------------------------
# a macro to determine the generator and ctest executable to use
# for testing. Simply to improve readability of the main script.
#-----------------------------------------------------------------------
macro(CMAKE_SETUP_TESTING)
  if(BUILD_TESTING)
    set(CMAKE_TEST_SYSTEM_LIBRARIES 0)
    foreach(util IN ITEMS CURL EXPAT ZLIB)
      if(CMAKE_USE_SYSTEM_${util})
        set(CMAKE_TEST_SYSTEM_LIBRARIES 1)
      endif()
    endforeach()

    # This variable is set by cmake, however to
    # test cmake we want to make sure that
    # the ctest from this cmake is used for testing
    # and not the ctest from the cmake building and testing
    # cmake.
    if(CMake_TEST_EXTERNAL_CMAKE)
      set(CMAKE_CTEST_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/ctest")
      set(CMAKE_CMAKE_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/cmake")
      set(CMAKE_CPACK_COMMAND "${CMake_TEST_EXTERNAL_CMAKE}/cpack")
      foreach(exe IN ITEMS cmake ctest cpack)
        add_executable(${exe} IMPORTED)
        set_property(TARGET ${exe} PROPERTY IMPORTED_LOCATION ${CMake_TEST_EXTERNAL_CMAKE}/${exe})
      endforeach()
    else()
      set(CMAKE_CTEST_COMMAND "${CMake_BIN_DIR}/ctest")
      set(CMAKE_CMAKE_COMMAND "${CMake_BIN_DIR}/cmake")
      set(CMAKE_CPACK_COMMAND "${CMake_BIN_DIR}/cpack")
    endif()
  endif()

  # configure some files for testing
  configure_file(Templates/CTestScript.cmake.in CTestScript.cmake @ONLY)
  configure_file(Tests/.NoDartCoverage Tests/.NoDartCoverage)
  configure_file(Tests/.NoDartCoverage Modules/.NoDartCoverage)
  configure_file(CTestCustom.cmake.in CTestCustom.cmake @ONLY)
endmacro()


# Provide a way for Visual Studio Express users to turn OFF the new FOLDER
# organization feature. Default to ON for non-Express users. Express users must
# explicitly turn off this option to build CMake in the Express IDE...
#
option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)


option(CMake_RUN_CLANG_TIDY "Run clang-tidy with the compiler." OFF)
if(CMake_RUN_CLANG_TIDY)
  if(CMake_SOURCE_DIR STREQUAL CMake_BINARY_DIR)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY requires an out-of-source build!")
  endif()
  find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
  if(NOT CLANG_TIDY_COMMAND)
    message(FATAL_ERROR "CMake_RUN_CLANG_TIDY is ON but clang-tidy is not found!")
  endif()
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

  option(CMake_USE_CLANG_TIDY_MODULE "Use CMake's clang-tidy module." OFF)
  if(CMake_USE_CLANG_TIDY_MODULE)
    find_library(CMake_CLANG_TIDY_MODULE NAMES cmake-clang-tidy-module DOC "Location of the clang-tidy module")
    if(NOT CMake_CLANG_TIDY_MODULE)
      message(FATAL_ERROR "CMake_USE_CLANG_TIDY_MODULE is ON but cmake-clang-tidy-module is not found!")
    endif()
    list(APPEND CMAKE_CXX_CLANG_TIDY "--load=${CMake_CLANG_TIDY_MODULE}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMake_CLANG_TIDY_MODULE}")
  endif()

  set(CMake_CLANG_TIDY_EXPORT_FIXES_DIR "" CACHE PATH "Directory to put clang-tidy fix files in.")
  mark_as_advanced(CMake_CLANG_TIDY_EXPORT_FIXES_DIR)
  if(CMake_CLANG_TIDY_EXPORT_FIXES_DIR)
    if(NOT IS_ABSOLUTE "${CMake_CLANG_TIDY_EXPORT_FIXES_DIR}")
      message(FATAL_ERROR "CMake_CLANG_TIDY_EXPORT_FIXES_DIR must be an absolute path!")
    endif()
    set(CMAKE_CXX_CLANG_TIDY_EXPORT_FIXES_DIR "${CMake_CLANG_TIDY_EXPORT_FIXES_DIR}")
  endif()

  # Create a preprocessor definition that depends on .clang-tidy content so
  # the compile command will change when .clang-tidy changes.  This ensures
  # that a subsequent build re-runs clang-tidy on all sources even if they
  # do not otherwise need to be recompiled.  Nothing actually uses this
  # definition.  We add it to targets on which we run clang-tidy just to
  # get the build dependency on the .clang-tidy file.
  file(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
  set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
  unset(clang_tidy_sha1)
  if(CMake_USE_CLANG_TIDY_MODULE)
    file(SHA1 "${CMake_CLANG_TIDY_MODULE}" clang_tidy_module_sha1)
    list(APPEND CLANG_TIDY_DEFINITIONS "CLANG_TIDY_MODULE_SHA1=${clang_tidy_module_sha1}")
    unset(clang_tidy_module_sha1)
  endif()

endif()
configure_file(.clang-tidy .clang-tidy COPYONLY)


option(CMake_RUN_IWYU "Run include-what-you-use with the compiler." OFF)
if(CMake_RUN_IWYU)
  find_program(IWYU_COMMAND NAMES include-what-you-use iwyu)
  if(NOT IWYU_COMMAND)
    message(FATAL_ERROR "CMake_RUN_IWYU is ON but include-what-you-use is not found!")
  endif()
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
    "${IWYU_COMMAND};-Xiwyu;--mapping_file=${CMake_SOURCE_DIR}/Utilities/IWYU/mapping.imp;-w")
  option(CMake_IWYU_VERBOSE "Run include-what-you-use in verbose mode" OFF)
  if (CMake_IWYU_VERBOSE)
    list(APPEND CMAKE_CXX_INCLUDE_WHAT_YOU_USE
      -Xiwyu -v7)
  endif ()
  list(APPEND CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${CMake_IWYU_OPTIONS})
endif()


#-----------------------------------------------------------------------
# a macro that only sets the FOLDER target property if it's
# "appropriate"
#-----------------------------------------------------------------------
macro(CMAKE_SET_TARGET_FOLDER tgt folder)
  if(CMAKE_USE_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    if(MSVC AND TARGET ${tgt})
      set_property(TARGET "${tgt}" PROPERTY FOLDER "${folder}")
    endif()
  else()
    set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
  endif()
endmacro()

#-----------------------------------------------------------------------
if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(CMAKE_CXX_PLATFORM_ID MATCHES "OpenBSD")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER}
      ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
      OUTPUT_VARIABLE _GXX_VERSION
    )
    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
      _GXX_VERSION_SHORT ${_GXX_VERSION})
    if(_GXX_VERSION_SHORT EQUAL 33)
      message(FATAL_ERROR
        "GXX 3.3 on OpenBSD is known to cause CPack to Crash.\n"
        "Please use GXX 4.2 or greater to build CMake on OpenBSD\n"
        "${CMAKE_CXX_COMPILER} version is: ${_GXX_VERSION}")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------
# The main section of the CMakeLists file
#
#-----------------------------------------------------------------------
include(Source/CMakeVersion.cmake)

include(CTest)

# Set up test-time configuration.
set_directory_properties(PROPERTIES
  TEST_INCLUDE_FILE "${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake")

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # where to write the resulting executables and libraries
  set(BUILD_SHARED_LIBS OFF)
  set(EXECUTABLE_OUTPUT_PATH "" CACHE INTERNAL "No configurable exe dir.")
  set(LIBRARY_OUTPUT_PATH "" CACHE INTERNAL
    "Where to put the libraries for CMake")

  # Load install destinations.
  include(Source/CMakeInstallDestinations.cmake)

  if(BUILD_TESTING)
    include(${CMake_SOURCE_DIR}/Tests/CMakeInstall.cmake)
  endif()

  # Checks for cmSystemTools.
  if(WIN32)
    set(HAVE_UNSETENV 0)
    set(HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE 1)
  else()
    include(CheckSymbolExists)
    CHECK_SYMBOL_EXISTS(unsetenv "stdlib.h" HAVE_UNSETENV)
    CHECK_SYMBOL_EXISTS(environ "stdlib.h" HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)
  endif()
endif()

# CMAKE_TESTS_CDASH_SERVER: CDash server used by CMake/Tests.
#
# If not defined or "", this variable defaults to the server at
# "http://open.cdash.org".
#
# If set explicitly to "NOTFOUND", curl tests and ctest tests that use
# the network are skipped.
#
# If set to something starting with "http://localhost/", the CDash is
# expected to be an instance of CDash used for CDash testing, pointing
# to a cdash4simpletest database. In these cases, the CDash dashboards
# should be run first.
#
if("x${CMAKE_TESTS_CDASH_SERVER}" STREQUAL "x" AND NOT CMake_TEST_NO_NETWORK)
  set(CMAKE_TESTS_CDASH_SERVER "http://open.cdash.org")
endif()

if(CMake_TEST_EXTERNAL_CMAKE)
  set(KWIML_TEST_ENABLE 1)
  add_subdirectory(Utilities/KWIML)
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  find_package(Threads)
  # build the utilities
  include(CMakeBuildUtilities)

  if(BUILD_QtDialog)
    if(APPLE)
      set(CMAKE_BUNDLE_VERSION
        "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
      set(CMAKE_BUNDLE_LOCATION "${CMAKE_INSTALL_PREFIX}")
      # make sure CMAKE_INSTALL_PREFIX ends in /
      if(NOT CMAKE_INSTALL_PREFIX MATCHES "/$")
        string(APPEND CMAKE_INSTALL_PREFIX "/")
      endif()
      string(APPEND CMAKE_INSTALL_PREFIX "CMake.app/Contents")
    endif()
  endif()

  if(UNIX)
    # Install executables with the RPATH set for libraries outside the build tree.
    # This is also suitable for binaries in the build tree.  Avoid re-link on install.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON CACHE BOOL "Install with RPATH set to find custom-built libraries.")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE BOOL "Build with RPATH set to match install-tree RPATH.")
    mark_as_advanced(CMAKE_INSTALL_RPATH_USE_LINK_PATH CMAKE_BUILD_WITH_INSTALL_RPATH)
  endif()

  # add the uninstall support
  configure_file(cmake_uninstall.cmake.in cmake_uninstall.cmake @ONLY)
  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

  include(CMakeCPack.cmake)

endif()

# setup some Testing support (a macro defined in this file)
CMAKE_SETUP_TESTING()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(NOT CMake_VERSION_IS_RELEASE)
    if((CMAKE_C_COMPILER_ID STREQUAL "GNU" AND
        NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS 4.2) OR
       (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND
        NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS 3.0 AND
        NOT "x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC") OR
       CMAKE_C_COMPILER_ID STREQUAL "AppleClang" OR
       CMAKE_C_COMPILER_ID STREQUAL "LCC")
      set(C_FLAGS_LIST -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts
                       -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security
                       -Wmissing-format-attribute -fno-common -Wundef
      )
      set(CXX_FLAGS_LIST -Wnon-virtual-dtor -Wcast-align -Wchar-subscripts -Wall -W
                         -Wshadow -Wpointer-arith -Wformat-security -Wundef
      )

      foreach(FLAG_LANG IN ITEMS C CXX)
        foreach(FLAG IN LISTS ${FLAG_LANG}_FLAGS_LIST)
          if(NOT " ${CMAKE_${FLAG_LANG}_FLAGS} " MATCHES " ${FLAG} ")
            string(APPEND CMAKE_${FLAG_LANG}_FLAGS " ${FLAG}")
          endif()
        endforeach()
      endforeach()

      unset(C_FLAGS_LIST)
      unset(CXX_FLAGS_LIST)
    endif()
  endif()

  # build the remaining subdirectories
  add_subdirectory(Source)
  add_subdirectory(Utilities)
endif()

add_subdirectory(Tests)

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  if(BUILD_TESTING)
    CMAKE_SET_TARGET_FOLDER(CMakeLibTests "Tests")
  endif()
  if(TARGET documentation)
    CMAKE_SET_TARGET_FOLDER(documentation "Documentation")
  endif()
endif()

if(BUILD_TESTING)
  add_test(SystemInformationNew "${CMAKE_CMAKE_COMMAND}"
    --system-information  -G "${CMAKE_GENERATOR}"
  )
endif()

if(NOT CMake_TEST_EXTERNAL_CMAKE)
  # Install license file as it requires.
  install(FILES Copyright.txt DESTINATION ${CMAKE_DOC_DIR})

  # Install script directories.
  install(
    DIRECTORY Help Modules Templates
    DESTINATION ${CMAKE_DATA_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                          GROUP_READ GROUP_EXECUTE
                          WORLD_READ WORLD_EXECUTE
    PATTERN "*.sh*" PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                                GROUP_READ GROUP_EXECUTE
                                WORLD_READ WORLD_EXECUTE
    REGEX "/(ExportImportList|cpp)$"
      PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                  GROUP_READ GROUP_EXECUTE
                  WORLD_READ WORLD_EXECUTE
    REGEX "Help/(dev|guide)($|/)" EXCLUDE
    )

  # Install auxiliary files integrating with other tools.
  add_subdirectory(Auxiliary)

  # Optionally sign installed binaries.
  if(CMake_INSTALL_SIGNTOOL)
    configure_file(Source/CMakeInstallSignTool.cmake.in Source/CMakeInstallSignTool.cmake @ONLY)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/Source/CMakeInstallSignTool.cmake)
  endif()
endif()
