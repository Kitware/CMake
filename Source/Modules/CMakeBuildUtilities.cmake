#-----------------------------------------------------------------------
# Build the utilities used by CMake
#
# Originally it was a macro in the root `CMakeLists.txt` with the comment
# "Simply to improve readability...".
# However, as part of the modernization refactoring it was moved into a
# separate file cuz adding library aliases wasn't possible inside the
# macro.
#-----------------------------------------------------------------------

# Suppress unnecessary checks in third-party code.
include(Utilities/cmThirdPartyChecks.cmake)

#---------------------------------------------------------------------
# Create the kwsys library for CMake.
set(KWSYS_NAMESPACE cmsys)
set(KWSYS_USE_SystemTools 1)
set(KWSYS_USE_Directory 1)
set(KWSYS_USE_RegularExpression 1)
set(KWSYS_USE_Base64 1)
set(KWSYS_USE_MD5 1)
set(KWSYS_USE_Process 1)
set(KWSYS_USE_CommandLineArguments 1)
set(KWSYS_USE_ConsoleBuf 1)
set(KWSYS_HEADER_ROOT ${CMake_BINARY_DIR}/Source)
set(KWSYS_INSTALL_DOC_DIR "${CMAKE_DOC_DIR}")
if(CMake_NO_CXX_STANDARD)
  set(KWSYS_CXX_STANDARD "")
endif()
if(CMake_NO_SELF_BACKTRACE)
  set(KWSYS_NO_EXECINFO 1)
endif()
if(WIN32)
  # FIXME: Teach KWSys to hard-code these checks on Windows.
  set(KWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC_COMPILED 0)
  set(KWSYS_C_HAS_PTRDIFF_T_COMPILED 1)
  set(KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H_COMPILED 1)
  set(KWSYS_CXX_HAS_RLIMIT64_COMPILED 0)
  set(KWSYS_CXX_HAS_SETENV_COMPILED 0)
  set(KWSYS_CXX_HAS_UNSETENV_COMPILED 0)
  set(KWSYS_CXX_HAS_UTIMENSAT_COMPILED 0)
  set(KWSYS_CXX_HAS_UTIMES_COMPILED 0)
  set(KWSYS_CXX_STAT_HAS_ST_MTIM_COMPILED 0)
  set(KWSYS_CXX_STAT_HAS_ST_MTIMESPEC_COMPILED 0)
  set(KWSYS_STL_HAS_WSTRING_COMPILED 1)
  set(KWSYS_SYS_HAS_IFADDRS_H 0)
endif()
add_subdirectory(Source/kwsys)
set(kwsys_folder "Utilities/KWSys")
CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE} "${kwsys_folder}")
CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}_c "${kwsys_folder}")
if(BUILD_TESTING)
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestDynload "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestProcess "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsC "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsCxx "${kwsys_folder}")
endif()

#---------------------------------------------------------------------
# Setup third-party libraries.
# Everything in the tree should be able to include files from the
# Utilities directory.
if((CMAKE_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_SYSTEM_NAME STREQUAL "OS400") AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using -isystem option generate error "template with C linkage"
  include_directories("${CMake_SOURCE_DIR}/Utilities/std")
else()
  include_directories(SYSTEM "${CMake_SOURCE_DIR}/Utilities/std")
endif()

include_directories("${CMake_BINARY_DIR}/Utilities")
if((CMAKE_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_SYSTEM_NAME STREQUAL "OS400") AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using -isystem option generate error "template with C linkage"
  include_directories("${CMake_SOURCE_DIR}/Utilities")
else()
  include_directories(SYSTEM "${CMake_SOURCE_DIR}/Utilities")
endif()

#---------------------------------------------------------------------
# Build CMake std library for CMake and CTest.
add_subdirectory(Utilities/std)
CMAKE_SET_TARGET_FOLDER(cmstd "Utilities/std")

# check for the use of system libraries versus builtin ones
# (a macro defined in this file)
CMAKE_HANDLE_SYSTEM_LIBRARIES()

if(CMAKE_USE_SYSTEM_KWIML)
  find_package(KWIML 1.0)
  if(NOT KWIML_FOUND)
    message(FATAL_ERROR "CMAKE_USE_SYSTEM_KWIML is ON but KWIML is not found!")
  endif()
else()
  if(BUILD_TESTING)
    set(KWIML_TEST_ENABLE 1)
  endif()
  add_subdirectory(Utilities/KWIML)
endif()

if(CMAKE_USE_SYSTEM_LIBRHASH)
  find_package(LibRHash)
  if(NOT LibRHash_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_LIBRHASH is ON but LibRHash is not found!")
  endif()
else()
  add_subdirectory(Utilities/cmlibrhash)
  add_library(LibRHash::LibRHash ALIAS cmlibrhash)
  CMAKE_SET_TARGET_FOLDER(cmlibrhash "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Build zlib library for Curl, CMake, and CTest.
if(CMAKE_USE_SYSTEM_ZLIB)
  find_package(ZLIB)
  if(NOT ZLIB_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_ZLIB is ON but a zlib is not found!")
  endif()
else()
  if(NOT POLICY CMP0102) # CMake < 3.17
    # Store in cache to protect from mark_as_advanced.
    set(ZLIB_INCLUDE_DIR ${CMake_SOURCE_DIR}/Utilities CACHE PATH "")
  else()
    set(ZLIB_INCLUDE_DIR ${CMake_SOURCE_DIR}/Utilities)
  endif()
  set(ZLIB_LIBRARY cmzlib)
  set(WITHOUT_ZLIB_DLL "")
  set(WITHOUT_ZLIB_DLL_WITH_LIB cmzlib)
  set(ZLIB_DLL "")
  set(ZLIB_DLL_WITH_LIB cmzlib)
  set(ZLIB_WINAPI "")
  set(ZLIB_WINAPI_COMPILED 0)
  set(ZLIB_WINAPI_WITH_LIB cmzlib)
  add_subdirectory(Utilities/cmzlib)
  add_library(ZLIB::ZLIB ALIAS cmzlib)
  CMAKE_SET_TARGET_FOLDER(cmzlib "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Build Curl library for CTest.
if(CMAKE_USE_SYSTEM_CURL)
  find_package(CURL)
  if(NOT CURL_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_CURL is ON but a curl is not found!")
  endif()
else()
  if(CMAKE_TESTS_CDASH_SERVER)
    set(CMAKE_CURL_TEST_URL "${CMAKE_TESTS_CDASH_SERVER}/user.php")
  endif()
  set(_CMAKE_USE_OPENSSL_DEFAULT OFF)
  if(NOT DEFINED CMAKE_USE_OPENSSL AND NOT WIN32 AND NOT APPLE
      AND CMAKE_SYSTEM_NAME MATCHES "(Linux|FreeBSD)")
    set(_CMAKE_USE_OPENSSL_DEFAULT ON)
  endif()
  option(CMAKE_USE_OPENSSL "Use OpenSSL." ${_CMAKE_USE_OPENSSL_DEFAULT})
  mark_as_advanced(CMAKE_USE_OPENSSL)
  if(CMAKE_USE_OPENSSL)
    set(CURL_CA_BUNDLE "" CACHE FILEPATH "Path to SSL CA Certificate Bundle")
    set(CURL_CA_PATH "" CACHE PATH "Path to SSL CA Certificate Directory")
    mark_as_advanced(CURL_CA_BUNDLE CURL_CA_PATH)
  endif()
  if(NOT CMAKE_USE_SYSTEM_NGHTTP2)
    # Tell curl's FindNGHTTP2 module to use our library.
    set(NGHTTP2_LIBRARY cmnghttp2)
    set(NGHTTP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmnghttp2/lib/includes)
  endif()
  add_subdirectory(Utilities/cmcurl)
  add_library(CURL::libcurl ALIAS cmcurl)
  CMAKE_SET_TARGET_FOLDER(cmcurl "Utilities/3rdParty")
  CMAKE_SET_TARGET_FOLDER(LIBCURL "Utilities/3rdParty")
  if(NOT CMAKE_USE_SYSTEM_NGHTTP2)
    # Configure after curl to reuse some check results.
    add_subdirectory(Utilities/cmnghttp2)
    CMAKE_SET_TARGET_FOLDER(cmnghttp2 "Utilities/3rdParty")
  endif()
endif()

#---------------------------------------------------------------------
# Build expat library for CMake, CTest, and libarchive.
if(CMAKE_USE_SYSTEM_EXPAT)
  find_package(EXPAT)
  if(NOT EXPAT_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_EXPAT is ON but a expat is not found!")
  endif()
  set(CMAKE_EXPAT_INCLUDES ${EXPAT_INCLUDE_DIRS})
  set(CMAKE_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
else()
  set(CMAKE_EXPAT_INCLUDES)
  set(CMAKE_EXPAT_LIBRARIES cmexpat)
  add_subdirectory(Utilities/cmexpat)
  add_library(EXPAT::EXPAT ALIAS cmexpat)
  CMAKE_SET_TARGET_FOLDER(cmexpat "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Build or use system libbz2 for libarchive.
if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
  if(CMAKE_USE_SYSTEM_BZIP2)
    find_package(BZip2)
  else()
    set(BZIP2_INCLUDE_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmbzip2")
    set(BZIP2_LIBRARIES cmbzip2)
    set(BZIP2_NEED_PREFIX "")
    set(USE_BZIP2_DLL "")
    set(USE_BZIP2_DLL_WITH_LIB cmbzip2)
    set(USE_BZIP2_STATIC "")
    set(USE_BZIP2_STATIC_WITH_LIB cmbzip2)
    add_subdirectory(Utilities/cmbzip2)
    CMAKE_SET_TARGET_FOLDER(cmbzip2 "Utilities/3rdParty")
  endif()
endif()

#---------------------------------------------------------------------
# Build or use system zstd for libarchive.
if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
  if(NOT CMAKE_USE_SYSTEM_ZSTD)
    set(ZSTD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmzstd")
    set(ZSTD_LIBRARY cmzstd)
    add_subdirectory(Utilities/cmzstd)
    CMAKE_SET_TARGET_FOLDER(cmzstd "Utilities/3rdParty")
  endif()
endif()

#---------------------------------------------------------------------
# Build or use system liblzma for libarchive.
if(NOT CMAKE_USE_SYSTEM_LIBARCHIVE)
  if(CMAKE_USE_SYSTEM_LIBLZMA)
    find_package(LibLZMA)
    if(NOT LIBLZMA_FOUND)
      message(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBLZMA is ON but LibLZMA is not found!")
    endif()
  else()
    add_subdirectory(Utilities/cmliblzma)
    CMAKE_SET_TARGET_FOLDER(cmliblzma "Utilities/3rdParty")
    set(LIBLZMA_HAS_AUTO_DECODER 1)
    set(LIBLZMA_HAS_EASY_ENCODER 1)
    set(LIBLZMA_HAS_LZMA_PRESET 1)
    set(LIBLZMA_INCLUDE_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmliblzma/liblzma/api")
    set(LIBLZMA_LIBRARY cmliblzma)
    set(HAVE_LZMA_STREAM_ENCODER_MT 1)
  endif()
endif()

#---------------------------------------------------------------------
# Build or use system libarchive for CMake and CTest.
if(CMAKE_USE_SYSTEM_LIBARCHIVE)
  find_package(LibArchive 3.3.3)
  if(NOT LibArchive_FOUND)
    message(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBARCHIVE is ON but LibArchive is not found!")
  endif()
  # NOTE `FindLibArchive` got imported targets support since 3.17
  if (NOT TARGET LibArchive::LibArchive)
    add_library(LibArchive::LibArchive UNKNOWN IMPORTED)
    set_target_properties(LibArchive::LibArchive PROPERTIES
      IMPORTED_LOCATION "${LibArchive_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${LibArchive_INCLUDE_DIRS}")
  endif ()
else()
  set(EXPAT_INCLUDE_DIR ${CMAKE_EXPAT_INCLUDES})
  set(EXPAT_LIBRARY ${CMAKE_EXPAT_LIBRARIES})
  set(ENABLE_MBEDTLS OFF)
  set(ENABLE_NETTLE OFF)
  if(DEFINED CMAKE_USE_OPENSSL)
    set(ENABLE_OPENSSL "${CMAKE_USE_OPENSSL}")
  else()
    set(ENABLE_OPENSSL OFF)
  endif()
  set(ENABLE_LIBB2 OFF)
  set(ENABLE_LZ4 OFF)
  set(ENABLE_LZO OFF)
  set(ENABLE_LZMA ON)
  set(ENABLE_ZSTD ON)
  set(ENABLE_ZLIB ON)
  set(ENABLE_BZip2 ON)
  set(ENABLE_LIBXML2 OFF)
  set(ENABLE_EXPAT OFF)
  set(ENABLE_PCREPOSIX OFF)
  set(ENABLE_LIBGCC OFF)
  set(ENABLE_CNG OFF)
  set(ENABLE_TAR OFF)
  set(ENABLE_TAR_SHARED OFF)
  set(ENABLE_CPIO OFF)
  set(ENABLE_CPIO_SHARED OFF)
  set(ENABLE_CAT OFF)
  set(ENABLE_CAT_SHARED OFF)
  set(ENABLE_UNZIP OFF)
  set(ENABLE_UNZIP_SHARED OFF)
  set(ENABLE_XATTR OFF)
  set(ENABLE_ACL OFF)
  set(ENABLE_ICONV OFF)
  set(ENABLE_TEST OFF)
  set(ENABLE_COVERAGE OFF)
  set(ENABLE_INSTALL OFF)
  set(POSIX_REGEX_LIB "" CACHE INTERNAL "libarchive: No POSIX regular expression support")
  set(ENABLE_SAFESEH "" CACHE INTERNAL "libarchive: No /SAFESEH linker flag")
  set(WINDOWS_VERSION "WIN7" CACHE INTERNAL "libarchive: Set Windows version to use (Windows only)")
  add_subdirectory(Utilities/cmlibarchive)
  add_library(LibArchive::LibArchive ALIAS cmlibarchive)
  target_compile_definitions(cmlibarchive INTERFACE LIBARCHIVE_STATIC)
  CMAKE_SET_TARGET_FOLDER(cmlibarchive "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Build jsoncpp library.
if(CMAKE_USE_SYSTEM_JSONCPP)
  find_package(JsonCpp 1.6.0)
  if(NOT JsonCpp_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_JSONCPP is ON but a JsonCpp is not found!")
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC|Clang")
    set_property(TARGET JsonCpp::JsonCpp APPEND PROPERTY
      INTERFACE_COMPILE_OPTIONS -Wno-deprecated-declarations)
  endif()
else()
  add_subdirectory(Utilities/cmjsoncpp)
  add_library(JsonCpp::JsonCpp ALIAS cmjsoncpp)
  CMAKE_SET_TARGET_FOLDER(cmjsoncpp "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Build libuv library.
if(CMAKE_USE_SYSTEM_LIBUV)
  if(WIN32)
    find_package(LibUV 1.38.0)
  else()
    find_package(LibUV 1.28.0)
  endif()
  if(NOT LIBUV_FOUND)
    message(FATAL_ERROR
      "CMAKE_USE_SYSTEM_LIBUV is ON but a libuv is not found!")
  endif()
else()
  add_subdirectory(Utilities/cmlibuv)
  add_library(LibUV::LibUV ALIAS cmlibuv)
  CMAKE_SET_TARGET_FOLDER(cmlibuv "Utilities/3rdParty")
endif()

#---------------------------------------------------------------------
# Use curses?
if(NOT DEFINED BUILD_CursesDialog)
  if(UNIX)
    include(${CMake_SOURCE_DIR}/Source/Checks/Curses.cmake)
    set(BUILD_CursesDialog_DEFAULT "${CMakeCheckCurses_COMPILED}")
  elseif(WIN32)
    set(BUILD_CursesDialog_DEFAULT "OFF")
  endif()
  option(BUILD_CursesDialog "Build the CMake Curses Dialog ccmake" "${BUILD_CursesDialog_DEFAULT}")
endif()
if(BUILD_CursesDialog)
  if(UNIX)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses)
    if(NOT CURSES_FOUND)
      message(WARNING
        "'ccmake' will not be built because Curses was not found.\n"
        "Turn off BUILD_CursesDialog to suppress this message."
        )
      set(BUILD_CursesDialog 0)
    endif()
  elseif(WIN32)
    # FIXME: Add support for system-provided pdcurses.
    add_subdirectory(Utilities/cmpdcurses)
    set(CURSES_LIBRARY cmpdcurses)
    set(CURSES_INCLUDE_PATH "") # cmpdcurses has usage requirements
    set(CMAKE_USE_SYSTEM_FORM 0)
    set(HAVE_CURSES_USE_DEFAULT_COLORS 1)
  endif()
endif()
if(BUILD_CursesDialog)
  if(NOT CMAKE_USE_SYSTEM_FORM)
    add_subdirectory(Source/CursesDialog/form)
  elseif(NOT CURSES_FORM_LIBRARY)
    message(FATAL_ERROR "CMAKE_USE_SYSTEM_FORM in ON but CURSES_FORM_LIBRARY is not set!")
  endif()
endif()

#---------------------------------------------------------------------
# Build cppdap library.
if(CMake_ENABLE_DEBUGGER)
  if(CMAKE_USE_SYSTEM_CPPDAP)
    find_package(cppdap CONFIG)
    if(NOT cppdap_FOUND)
      message(FATAL_ERROR
        "CMAKE_USE_SYSTEM_CPPDAP is ON but a cppdap is not found!")
    endif()
  else()
    add_subdirectory(Utilities/cmcppdap)
    add_library(cppdap::cppdap ALIAS cmcppdap)
    CMAKE_SET_TARGET_FOLDER(cppdap "Utilities/3rdParty")
  endif()
endif()
