cmake_minimum_required(VERSION 2.6.3 FATAL_ERROR)
#
#
project(libarchive C)
#
set(CMAKE_MODULE_PATH "${libarchive_SOURCE_DIR}/build/cmake")

# We use CHECK_TYPE_SIZE for existence tests, so use only one arch.
list(LENGTH CMAKE_OSX_ARCHITECTURES NUM_ARCHS)
if(${NUM_ARCHS} GREATER 1)
  list(GET CMAKE_OSX_ARCHITECTURES 0 CMAKE_TRY_COMPILE_OSX_ARCHITECTURES)
endif()

#
# Version - read from 'version' file.
#
file(STRINGS ${libarchive_SOURCE_DIR}/build/version _version)
string(REGEX REPLACE
 "^([0-9])[0-9][0-9][0-9][0-9][0-9][0-9][a-z]?$" "\\1" _major ${_version})
string(REGEX REPLACE
 "^[0-9]([0-9][0-9][0-9])[0-9][0-9][0-9][a-z]?$" "\\1" _minor ${_version})
string(REGEX REPLACE
 "^[0-9][0-9][0-9][0-9]([0-9][0-9][0-9])[a-z]?$" "\\1" _revision ${_version})
string(REGEX REPLACE
 "^[0-9][0-9][0-9][0-9][0-9][0-9][0-9]([a-z]?)$" "\\1" _quality ${_version})
set(_version_number ${_major}${_minor}${_revision})
string(REGEX REPLACE "[0]*([^0][0-9]*)$" "\\1" _minor ${_minor})
string(REGEX REPLACE "[0]*([^0][0-9]*)$" "\\1" _revision ${_revision})
#
set(VERSION                    "${_major}.${_minor}.${_revision}${_quality}")
set(BSDCPIO_VERSION_STRING     "${VERSION}")
set(BSDTAR_VERSION_STRING      "${VERSION}")
set(LIBARCHIVE_VERSION_NUMBER  "${_version_number}")
set(LIBARCHIVE_VERSION_STRING  "${VERSION}")
# Shared library number
set(SOVERSION                   8)

option(ENABLE_TAR "Enable tar building" ON)
option(ENABLE_TAR_SHARED "Enable dynamic build of tar" OFF)
option(ENABLE_CPIO "Enable cpio building" ON)
option(ENABLE_CPIO_SHARED "Enable dynamic build of cpio" OFF)
option(ENABLE_XATTR "Enable extended attribute support" ON)
option(ENABLE_ACL "Enable ACL support" OFF)
option(ENABLE_TEST "Enable unit and regression tests" OFF)

if(ENABLE_TEST)
    enable_testing()
endif(ENABLE_TEST)
if(WIN32 AND NOT CYGWIN)
  # Currently, dynamic build only.
  set(ENABLE_TAR_SHARED ON)
  set(ENABLE_CPIO_SHARED ON)
endif(WIN32 AND NOT CYGWIN)

if(WIN32)
  set(_WIN32_WINNT 0x0500 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0500 for Windows 2000 APIs")
  set(WINVER 0x0500 CACHE INTERNAL "Setting WINVER to 0x0500 for Windows 2000 APIs")
endif(WIN32)

#
include(CheckCSourceRuns)
include(CheckFileOffsetBits)
include(CheckFuncs)
include(CheckHeaderDirent)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckStructMember)
include(CheckSymbolExists)
include(CheckTypeExists)
include(CheckTypeSize)

#
# Generate list.h
#
macro(GENERATE_LIST_H _listfile _cmlist __list_sources)
  set(_argv ${ARGV})
  # Remove _listfile and _cmlist from _argv
  list(REMOVE_AT _argv 0 1)
  if(NOT EXISTS "${_listfile}" OR
     ${_cmlist} IS_NEWER_THAN "${_listfile}")

    message(STATUS "Generating ${_listfile}")
    file(WRITE ${_listfile} "")
    foreach(testfile ${_argv})
      if(testfile MATCHES "^test_[^/]+[.]c$")
        file(STRINGS ${testfile} testvar REGEX "^DEFINE_TEST")
        foreach(deftest ${testvar})
          file(APPEND ${_listfile} "${deftest}\n")
        endforeach(deftest)
      endif(testfile MATCHES "^test_[^/]+[.]c$")
    endforeach(testfile)

  endif(NOT EXISTS "${_listfile}" OR
     ${_cmlist} IS_NEWER_THAN "${_listfile}")
endmacro(GENERATE_LIST_H)
#
# Generate installation rules for man pages.
#
macro(INSTALL_MAN __mans)
  foreach(_man ${ARGV})
    string(REGEX REPLACE "^.+[.]([1-9])" "\\1" _mansect ${_man})
    install(FILES ${_man} DESTINATION "share/man/man${_mansect}")
  endforeach(_man)
endmacro(INSTALL_MAN __mans)

#
# Check compress/decompress libraries
#
if(WIN32 AND NOT CMAKE_CL_64 AND NOT CYGWIN)
  # GnuWin32 is only for Win32, not Win64.
  set(__GNUWIN32PATH "C:/Program Files/GnuWin32")
endif(WIN32 AND NOT CMAKE_CL_64 AND NOT CYGWIN)
if(DEFINED __GNUWIN32PATH AND EXISTS "${__GNUWIN32PATH}")
  # You have to add a path availabel DLL file into PATH environment variable.
  # Maybe DLL path is "C:/Program Files/GnuWin32/bin".
  # The zlib and the bzip2 Setup program have installed programs and DLLs into
  # "C:/Program Files/GnuWin32" by default.
  # This is convenience setting for Windows.
  set(CMAKE_PREFIX_PATH ${__GNUWIN32PATH} $(CMAKE_PREFIX_PATH))
  #
  # If you didn't use Setup program or installed into nonstandard path,
  # cmake cannot find out your zlib or bzip2 libraries and include files,
  # you should execute cmake with  -DCMAKE_PREFIX_PATH option.
  #   e.g.
  #     cmake -DCMAKE_PREFIX_PATH=<your-GnuWin32-path> <path-to-source>
  #
  # If compiling error occured in zconf.h, You may need patch to zconf.h.
  #--- zconf.h.orig 2005-07-21 00:40:26.000000000
  #+++ zconf.h  2009-01-19 11:39:10.093750000
  #@@ -286,7 +286,7 @@
  #
  # #if 1           /* HAVE_UNISTD_H -- this line is updated by ./configure */
  # #  include <sys/types.h> /* for off_t */
  #-#  include <unistd.h>    /* for SEEK_* and off_t */
  #+#  include <stdio.h>    /* for SEEK_* and off_t */
  # #  ifdef VMS
  # #    include <unixio.h>   /* for off_t */
  # #  endif
endif(DEFINED __GNUWIN32PATH AND EXISTS "${__GNUWIN32PATH}")

set(ADDITIONAL_LIBS "")

#
# Find ZLIB
#
find_package(ZLIB)
if(ZLIB_FOUND)
  set(HAVE_LIBZ 1)
  set(HAVE_ZLIB_H 1)
  include_directories(${ZLIB_INCLUDE_DIR})
  # if building inside cmake do not add this lib
  # as it will not exist at try compile time
  if(NOT "${ZLIB_LIBRARIES}" MATCHES cmzlib)
    list(APPEND ADDITIONAL_LIBS ${ZLIB_LIBRARIES})
  endif(NOT "${ZLIB_LIBRARIES}" MATCHES cmzlib)
endif(ZLIB_FOUND)
mark_as_advanced(CLEAR ZLIB_INCLUDE_DIR)
mark_as_advanced(CLEAR ZLIB_LIBRARY)
#
# Find BZip2
#
find_package(BZip2)
if(BZIP2_FOUND)
  set(HAVE_LIBBZ2 1)
  set(HAVE_BZLIB_H 1)
  include_directories(${BZIP2_INCLUDE_DIR})
  # if building inside cmake do not add this lib
  # as it will not exist at try compile time
  if(NOT "${BZIP2_LIBRARIES}" MATCHES cmbzip2)
    list(APPEND ADDITIONAL_LIBS ${BZIP2_LIBRARIES})
  endif()
endif(BZIP2_FOUND)
mark_as_advanced(CLEAR BZIP2_INCLUDE_DIR)
mark_as_advanced(CLEAR BZIP2_LIBRARIES)
#
# Find LZMA
#
if(BUILD_ARCHIVE_WITHIN_CMAKE)
  # do not depend on external library
  # for internal CMake build
  set(LZMA_FOUND FALSE)
else(BUILD_ARCHIVE_WITHIN_CMAKE)
  find_package(LZMA)
endif(BUILD_ARCHIVE_WITHIN_CMAKE)

if(LZMA_FOUND)
  set(HAVE_LIBLZMA 1)
  set(HAVE_LZMA_H 1)
  include_directories(${LZMA_INCLUDE_DIR})
  list(APPEND ADDITIONAL_LIBS ${LZMA_LIBRARIES})
  mark_as_advanced(CLEAR LZMA_INCLUDE_DIR)
  mark_as_advanced(CLEAR LZMA_LIBRARY)
elseif(LZMADEC_FOUND)
  set(HAVE_LIBLZMADEC 1)
  set(HAVE_LZMADEC_H 1)
  include_directories(${LZMADEC_INCLUDE_DIR})
  list(APPEND ADDITIONAL_LIBS ${LZMADEC_LIBRARIES})
  mark_as_advanced(CLEAR LZMADEC_INCLUDE_DIR)
  mark_as_advanced(CLEAR LZMADEC_LIBRARY)
else(LZMA_FOUND)
  mark_as_advanced(CLEAR LZMA_INCLUDE_DIR)
  mark_as_advanced(CLEAR LZMA_LIBRARY)
endif(LZMA_FOUND)

#
# Check headers
#
CHECK_HEADER_DIRENT()

set(INCLUDES "")
macro(LA_CHECK_INCLUDE_FILE header var)
      CHECK_INCLUDE_FILES("${INCLUDES};${header}" ${var})
      if(${var})
         set(INCLUDES ${INCLUDES} ${header})
      endif(${var})
endmacro(LA_CHECK_INCLUDE_FILE)

# Few headers that must precede other headers
# Must precede sys/extattr.h on FreeBSD
LA_CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)

# Alphabetize the rest unless there's a compelling reason
LA_CHECK_INCLUDE_FILE("acl/libacl.h" HAVE_ACL_LIBACL_H)
LA_CHECK_INCLUDE_FILE("attr/xattr.h" HAVE_ATTR_XATTR_H)
LA_CHECK_INCLUDE_FILE("ctype.h" HAVE_CTYPE_H)
LA_CHECK_INCLUDE_FILE("direct.h" HAVE_DIRECT_H)
LA_CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
LA_CHECK_INCLUDE_FILE("errno.h" HAVE_ERRNO_H)
LA_CHECK_INCLUDE_FILE("ext2fs/ext2_fs.h" HAVE_EXT2FS_EXT2_FS_H)
LA_CHECK_INCLUDE_FILE("fcntl.h" HAVE_FCNTL_H)
LA_CHECK_INCLUDE_FILE("grp.h" HAVE_GRP_H)
LA_CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
LA_CHECK_INCLUDE_FILE("io.h" HAVE_IO_H)
LA_CHECK_INCLUDE_FILE("langinfo.h" HAVE_LANGINFO_H)
LA_CHECK_INCLUDE_FILE("limits.h" HAVE_LIMITS_H)
LA_CHECK_INCLUDE_FILE("linux/fs.h" HAVE_LINUX_FS_H)
LA_CHECK_INCLUDE_FILE("locale.h" HAVE_LOCALE_H)
LA_CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)
LA_CHECK_INCLUDE_FILE("paths.h" HAVE_PATHS_H)
LA_CHECK_INCLUDE_FILE("poll.h" HAVE_POLL_H)
LA_CHECK_INCLUDE_FILE("process.h" HAVE_PROCESS_H)
LA_CHECK_INCLUDE_FILE("pwd.h" HAVE_PWD_H)
LA_CHECK_INCLUDE_FILE("regex.h" HAVE_REGEX_H)
LA_CHECK_INCLUDE_FILE("signal.h" HAVE_SIGNAL_H)
LA_CHECK_INCLUDE_FILE("stdarg.h" HAVE_STDARG_H)
LA_CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
LA_CHECK_INCLUDE_FILE("stdlib.h" HAVE_STDLIB_H)
LA_CHECK_INCLUDE_FILE("string.h" HAVE_STRING_H)
LA_CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)
LA_CHECK_INCLUDE_FILE("sys/acl.h" HAVE_SYS_ACL_H)
LA_CHECK_INCLUDE_FILE("sys/extattr.h" HAVE_SYS_EXTATTR_H)
LA_CHECK_INCLUDE_FILE("sys/ioctl.h" HAVE_SYS_IOCTL_H)
LA_CHECK_INCLUDE_FILE("sys/mkdev.h" HAVE_SYS_MKDEV_H)
LA_CHECK_INCLUDE_FILE("sys/param.h" HAVE_SYS_PARAM_H)
LA_CHECK_INCLUDE_FILE("sys/poll.h" HAVE_SYS_POLL_H)
LA_CHECK_INCLUDE_FILE("sys/select.h" HAVE_SYS_SELECT_H)
LA_CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
LA_CHECK_INCLUDE_FILE("sys/time.h" HAVE_SYS_TIME_H)
LA_CHECK_INCLUDE_FILE("sys/utime.h" HAVE_SYS_UTIME_H)
LA_CHECK_INCLUDE_FILE("sys/wait.h" HAVE_SYS_WAIT_H)
LA_CHECK_INCLUDE_FILE("time.h" HAVE_TIME_H)
LA_CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)
LA_CHECK_INCLUDE_FILE("utime.h" HAVE_UTIME_H)
LA_CHECK_INCLUDE_FILE("wchar.h" HAVE_WCHAR_H)
LA_CHECK_INCLUDE_FILE("wctype.h" HAVE_WCTYPE_H)
LA_CHECK_INCLUDE_FILE("windows.h" HAVE_WINDOWS_H)


#
# Some headers require extra includes when they're available.
#

#
# Find OpenSSL
#
if(CMAKE_USE_OPENSSL)
  find_package(OpenSSL)
else()
  set(OPENSSL_FOUND 0)
endif()
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  list(APPEND ADDITIONAL_LIBS ${OPENSSL_LIBRARIES})
endif(OPENSSL_FOUND)
#
# Check MD5/RMD160/SHA headers
#
LA_CHECK_INCLUDE_FILE("md5.h" HAVE_MD5_H)
LA_CHECK_INCLUDE_FILE("openssl/md5.h" HAVE_OPENSSL_MD5_H)
LA_CHECK_INCLUDE_FILE("openssl/ripemd.h" HAVE_OPENSSL_RIPEMD_H)
LA_CHECK_INCLUDE_FILE("openssl/sha.h" HAVE_OPENSSL_SHA_H)
LA_CHECK_INCLUDE_FILE("ripemd.h" HAVE_RIPEMD_H)
LA_CHECK_INCLUDE_FILE("rmd160.h" HAVE_RMD160_H)
LA_CHECK_INCLUDE_FILE("sha.h" HAVE_SHA_H)
LA_CHECK_INCLUDE_FILE("sha1.h" HAVE_SHA1_H)
LA_CHECK_INCLUDE_FILE("sha2.h" HAVE_SHA2_H)
LA_CHECK_INCLUDE_FILE("sha256.h" HAVE_SHA256_H)

#
# Find MD5/RMD160/SHA library
#
if(CMAKE_USE_OPENSSL)
  find_library(CRYPTO_LIBRARY NAMES crypto)
else()
  set(CRYPTO_LIBRARY "")
endif()
if(CRYPTO_LIBRARY)
  list(APPEND ADDITIONAL_LIBS ${CRYPTO_LIBRARY})
else(CRYPTO_LIBRARY)
  if(NOT OPENSSL_FOUND)
    find_library(MD_LIBRARY NAMES md)
    if(MD_LIBRARY)
      list(APPEND ADDITIONAL_LIBS ${MD_LIBRARY})
    endif(MD_LIBRARY)
  endif(NOT OPENSSL_FOUND)
endif(CRYPTO_LIBRARY)
#
# Check MD5/RMD160/SHA functions
#
set(CMAKE_REQUIRED_LIBRARIES ${ADDITIONAL_LIBS})
CHECK_FUNCTION_EXISTS(MD5_Init HAVE_MD5_Init)
if(NOT HAVE_MD5_Init)
  CHECK_FUNCTION_EXISTS(MD5Init HAVE_MD5Init)
  if(HAVE_MD5Init)
    set(MD5_Init, "MD5Init")
    set(MD5_Update, "MD5Update")
    set(MD5_Final, "MD5Final")
  endif(HAVE_MD5Init)
endif(NOT HAVE_MD5_Init)
if(HAVE_MD5_Init OR HAVE_MD5Init)
  set(HAVE_MD5 1)
endif(HAVE_MD5_Init OR HAVE_MD5Init)
#
CHECK_FUNCTION_EXISTS(RIPEMD160_Init HAVE_RIPEMD160_Init)
if(NOT HAVE_RIPEMD160_Init)
  CHECK_FUNCTION_EXISTS(RMD160Init HAVE_RMD160Init)
  if(HAVE_RMD160Init)
    set(RIPEMD160_Init, "RMD160Init")
    set(RIPEMD160_Update, "RMD160Update")
    set(RIPEMD160_Final, "RMD160Final")
  endif(HAVE_RMD160Init)
endif(NOT HAVE_RIPEMD160_Init)
if(HAVE_RIPEMD160_Init OR HAVE_RMD160Init)
  set(HAVE_RMD160 1)
endif(HAVE_RIPEMD160_Init OR HAVE_RMD160Init)
#
CHECK_FUNCTION_EXISTS(SHA1_Init HAVE_SHA1_Init)
if(NOT HAVE_SHA1_Init)
  CHECK_FUNCTION_EXISTS(SHA1Init HAVE_SHA1Init)
  if(HAVE_SHA1Init)
    set(SHA1_Init, "SHA1Init")
    set(SHA1_Update, "SHA1Update")
    set(SHA1_Final, "SHA1Final")
  endif(HAVE_SHA1Init)
endif(NOT HAVE_SHA1_Init)
if(HAVE_SHA1_Init OR HAVE_SHA1Init)
  set(HAVE_SHA1 1)
endif(HAVE_SHA1_Init OR HAVE_SHA1Init)
#
CHECK_FUNCTION_EXISTS(SHA256_Init HAVE_SHA256)
CHECK_FUNCTION_EXISTS(SHA384_Init HAVE_SHA384)
CHECK_FUNCTION_EXISTS(SHA512_Init HAVE_SHA512)

#
# Check functions
#
CHECK_SYMBOL_EXISTS(CreateHardLinkA "windows.h" HAVE_CREATEHARDLINKA)
CHECK_SYMBOL_EXISTS(CreateHardLinkW "windows.h" HAVE_CREATEHARDLINKW)
CHECK_FUNCTION_EXISTS_GLIBC(chflags HAVE_CHFLAGS)
CHECK_FUNCTION_EXISTS_GLIBC(chown HAVE_CHOWN)
CHECK_FUNCTION_EXISTS_GLIBC(chroot HAVE_CHROOT)
CHECK_FUNCTION_EXISTS_GLIBC(fchdir HAVE_FCHDIR)
CHECK_FUNCTION_EXISTS_GLIBC(fchflags HAVE_FCHFLAGS)
CHECK_FUNCTION_EXISTS_GLIBC(fchmod HAVE_FCHMOD)
CHECK_FUNCTION_EXISTS_GLIBC(fchown HAVE_FCHOWN)
CHECK_FUNCTION_EXISTS_GLIBC(fcntl HAVE_FCNTL)
CHECK_FUNCTION_EXISTS_GLIBC(fork HAVE_FORK)
CHECK_FUNCTION_EXISTS_GLIBC(fstat HAVE_FSTAT)
CHECK_FUNCTION_EXISTS_GLIBC(ftruncate HAVE_FTRUNCATE)
CHECK_FUNCTION_EXISTS_GLIBC(futimes HAVE_FUTIMES)
CHECK_FUNCTION_EXISTS_GLIBC(geteuid HAVE_GETEUID)
CHECK_FUNCTION_EXISTS_GLIBC(getpid HAVE_GETPID)
CHECK_FUNCTION_EXISTS_GLIBC(lchflags HAVE_LCHFLAGS)
CHECK_FUNCTION_EXISTS_GLIBC(lchmod HAVE_LCHMOD)
CHECK_FUNCTION_EXISTS_GLIBC(lchown HAVE_LCHOWN)
CHECK_FUNCTION_EXISTS_GLIBC(link HAVE_LINK)
CHECK_FUNCTION_EXISTS_GLIBC(lstat HAVE_LSTAT)
CHECK_FUNCTION_EXISTS_GLIBC(lutimes HAVE_LUTIMES)
CHECK_FUNCTION_EXISTS_GLIBC(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS_GLIBC(mkdir HAVE_MKDIR)
CHECK_FUNCTION_EXISTS_GLIBC(mkfifo HAVE_MKFIFO)
CHECK_FUNCTION_EXISTS_GLIBC(mknod HAVE_MKNOD)
CHECK_FUNCTION_EXISTS_GLIBC(nl_langinfo HAVE_NL_LANGINFO)
CHECK_FUNCTION_EXISTS_GLIBC(pipe HAVE_PIPE)
CHECK_FUNCTION_EXISTS_GLIBC(poll HAVE_POLL)
CHECK_FUNCTION_EXISTS_GLIBC(readlink HAVE_READLINK)
CHECK_FUNCTION_EXISTS_GLIBC(select HAVE_SELECT)
CHECK_FUNCTION_EXISTS_GLIBC(setenv HAVE_SETENV)
CHECK_FUNCTION_EXISTS_GLIBC(setlocale HAVE_SETLOCALE)
CHECK_FUNCTION_EXISTS_GLIBC(strchr HAVE_STRCHR)
CHECK_FUNCTION_EXISTS_GLIBC(strdup HAVE_STRDUP)
CHECK_FUNCTION_EXISTS_GLIBC(strerror HAVE_STRERROR)
CHECK_FUNCTION_EXISTS_GLIBC(strncpy_s HAVE_STRNCPY_S)
CHECK_FUNCTION_EXISTS_GLIBC(strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS_GLIBC(symlink HAVE_SYMLINK)
CHECK_FUNCTION_EXISTS_GLIBC(timegm HAVE_TIMEGM)
CHECK_FUNCTION_EXISTS_GLIBC(tzset HAVE_TZSET)
CHECK_FUNCTION_EXISTS_GLIBC(unsetenv HAVE_UNSETENV)
CHECK_FUNCTION_EXISTS_GLIBC(utime HAVE_UTIME)
CHECK_FUNCTION_EXISTS_GLIBC(utimes HAVE_UTIMES)
CHECK_FUNCTION_EXISTS_GLIBC(vfork HAVE_VFORK)
CHECK_FUNCTION_EXISTS_GLIBC(wcrtomb HAVE_WCRTOMB)
CHECK_FUNCTION_EXISTS_GLIBC(wcscpy HAVE_WCSCPY)
CHECK_FUNCTION_EXISTS_GLIBC(wcslen HAVE_WCSLEN)
CHECK_FUNCTION_EXISTS_GLIBC(wctomb HAVE_WCTOMB)
CHECK_SYMBOL_EXISTS(wmemcmp "wchar.h" HAVE_WMEMCMP)
CHECK_SYMBOL_EXISTS(wmemcpy "wchar.h" HAVE_WMEMCPY)

set(CMAKE_REQUIRED_LIBRARIES "")
CHECK_SYMBOL_EXISTS(fseeko           "stdio.h"    HAVE_FSEEKO)
CHECK_SYMBOL_EXISTS(strerror_r       "string.h"   HAVE_STRERROR_R)
CHECK_SYMBOL_EXISTS(strftime         "time.h"     HAVE_STRFTIME)
CHECK_SYMBOL_EXISTS(vprintf          "stdio.h"    HAVE_VPRINTF)

CHECK_C_SOURCE_COMPILES(
  "#include <sys/mkdev.h>\nint main() { return major(256); }"
  MAJOR_IN_MKDEV)
CHECK_C_SOURCE_COMPILES(
  "#include <sys/sysmacros.h>\nint main() { return major(256); }"
  MAJOR_IN_SYSMACROS)

if(HAVE_STRERROR_R)
  set(HAVE_DECL_STRERROR_R 1)
endif(HAVE_STRERROR_R)

#
# Check defines
#
set(headers "limits.h")
if(HAVE_STDINT_H)
  list(APPEND headers "stdint.h")
endif(HAVE_STDINT_H)
if(HAVE_INTTYPES_H)
  list(APPEND headers "inttypes.h")
endif(HAVE_INTTYPES_H)
CHECK_SYMBOL_EXISTS(EFTYPE           "errno.h"    HAVE_EFTYPE)
CHECK_SYMBOL_EXISTS(EILSEQ           "errno.h"    HAVE_EILSEQ)
CHECK_SYMBOL_EXISTS(D_MD_ORDER       "langinfo.h" HAVE_D_MD_ORDER)
CHECK_SYMBOL_EXISTS(optarg           "unistd.h"   HAVE_DECL_OPTARG)
CHECK_SYMBOL_EXISTS(optind           "unistd.h"   HAVE_DECL_OPTIND)
CHECK_SYMBOL_EXISTS(INT64_MAX        "${headers}" HAVE_DECL_INT64_MAX)
CHECK_SYMBOL_EXISTS(INT64_MIN        "${headers}" HAVE_DECL_INT64_MIN)
CHECK_SYMBOL_EXISTS(UINT32_MAX       "${headers}" HAVE_DECL_UINT32_MAX)
CHECK_SYMBOL_EXISTS(UINT64_MAX       "${headers}" HAVE_DECL_UINT64_MAX)
CHECK_SYMBOL_EXISTS(SIZE_MAX         "${headers}" HAVE_DECL_SIZE_MAX)
CHECK_SYMBOL_EXISTS(SSIZE_MAX        "limits.h"   HAVE_DECL_SSIZE_MAX)

#
# Check struct members
#
# Check for birthtime in struct stat
CHECK_STRUCT_MEMBER("struct stat" st_birthtime
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_BIRTHTIME)

# Check for high-resolution timestamps in struct stat
CHECK_STRUCT_MEMBER("struct stat" st_birthtimespec.tv_nsec
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC_TV_NSEC)
CHECK_STRUCT_MEMBER("struct stat" st_mtimespec.tv_nsec
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_MTIMESPEC_TV_NSEC)
CHECK_STRUCT_MEMBER("struct stat" st_mtim.tv_nsec
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC)
CHECK_STRUCT_MEMBER("struct stat" st_mtime_n
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_MTIME_N)
CHECK_STRUCT_MEMBER("struct stat" st_umtime
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_UMTIME)
CHECK_STRUCT_MEMBER("struct stat" st_mtime_usec
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_MTIME_USEC)
# Check for block size support in struct stat
CHECK_STRUCT_MEMBER("struct stat" st_blksize
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_BLKSIZE)
# Check for st_flags in struct stat (BSD fflags)
CHECK_STRUCT_MEMBER("struct stat" st_flags
    "sys/types.h;sys/stat.h" HAVE_STRUCT_STAT_ST_FLAGS)
#
#
CHECK_STRUCT_MEMBER("struct tm" tm_sec
    "sys/types.h;sys/time.h;time.h" TIME_WITH_SYS_TIME)

#
# Check for integer types
#
# XXX There must be a way to make this simpler <sigh> XXXX
#
CHECK_TYPE_SIZE("long long int"            LONG_LONG_INT)
CHECK_TYPE_SIZE("unsigned long long"       UNSIGNED_LONG_LONG)
CHECK_TYPE_SIZE("unsigned long long int"   UNSIGNED_LONG_LONG_INT)

#
CHECK_TYPE_SIZE(dev_t       DEV_T)
if(NOT HAVE_DEV_T)
  set(dev_t "unsigned int")
endif(NOT HAVE_DEV_T)
#
CHECK_TYPE_SIZE(gid_t       GID_T)
if(NOT HAVE_GID_T)
  if(WIN32)
    set(gid_t "short")
  else(WIN32)
    set(gid_t "unsigned int")
  endif(WIN32)
endif(NOT HAVE_GID_T)
#
CHECK_TYPE_SIZE(id_t        ID_T)
if(NOT HAVE_ID_T)
  if(WIN32)
    set(id_t "short")
  else(WIN32)
    set(id_t "unsigned int")
  endif(WIN32)
endif(NOT HAVE_ID_T)
#
CHECK_TYPE_SIZE(int64_t     INT64_T)
if(NOT HAVE_INT64_T)
  if(WIN32)
    set(int64_t "__int64")
  else(WIN32)
    set(int64_t "long long")
  endif(WIN32)
endif(NOT HAVE_INT64_T)
#
CHECK_TYPE_SIZE(intmax_t    INTMAX_T)
if(NOT HAVE_INTMAX_T)
  set(intmax_t "int64_t")
endif(NOT HAVE_INTMAX_T)
#
CHECK_TYPE_SIZE(mode_t      MODE_T)
if(NOT HAVE_MODE_T)
  if(WIN32)
    set(mode_t "unsigned short")
  else(WIN32)
    set(mode_t "int")
  endif(WIN32)
endif(NOT HAVE_MODE_T)
#
CHECK_TYPE_SIZE(off_t       OFF_T)
if(NOT HAVE_OFF_T)
  set(off_t "__int64")
endif(NOT HAVE_OFF_T)
#
CHECK_TYPE_SIZE(size_t      SIZE_T)
if(NOT HAVE_SIZE_T)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(size_t "uint64_t")
  else("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(size_t   "uint32_t")
  endif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
endif(NOT HAVE_SIZE_T)
#
CHECK_TYPE_SIZE(ssize_t     SSIZE_T)
if(NOT HAVE_SSIZE_T)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(ssize_t "int64_t")
  else("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(ssize_t "long")
  endif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
endif(NOT HAVE_SSIZE_T)
#
CHECK_TYPE_SIZE(uid_t       UID_T)
if(NOT HAVE_UID_T)
  if(WIN32)
    set(uid_t "short")
  else(WIN32)
    set(uid_t "unsigned int")
  endif(WIN32)
endif(NOT HAVE_UID_T)
#
CHECK_TYPE_SIZE(uint16_t    UINT16_T)
if(NOT HAVE_UINT16_T)
  set(uint16_t "unsigned short")
endif(NOT HAVE_UINT16_T)
#
CHECK_TYPE_SIZE(uint32_t    UINT32_T)
if(NOT HAVE_UINT32_T)
  set(uint32_t "unsigned int")
endif(NOT HAVE_UINT32_T)
CHECK_TYPE_SIZE(int32_t    INT32_T)
if(NOT HAVE_INT32_T)
  set(int32_t "int")
endif(NOT HAVE_INT32_T)
#
CHECK_TYPE_SIZE(uint64_t    UINT64_T)
if(NOT HAVE_UINT64_T)
  if(WIN32)
    set(uint64_t "unsigned __int64")
  else(WIN32)
    set(uint64_t "unsigned long long")
  endif(WIN32)
endif(NOT HAVE_UINT64_T)
#
CHECK_TYPE_SIZE(uintmax_t   UINTMAX_T)
if(NOT HAVE_UINTMAX_T)
  set(uintmax_t "uint64_t")
endif(NOT HAVE_UINTMAX_T)
#
CHECK_TYPE_SIZE(intptr_t   INTPTR_T)
if(NOT HAVE_INTPTR_T)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(intptr_t "int64_t")
  else()
    set(intptr_t "int32_t")
  endif()
endif(NOT HAVE_INTPTR_T)
#
CHECK_TYPE_SIZE(uintptr_t   UINTPTR_T)
if(NOT HAVE_UINTPTR_T)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(uintptr_t "uint64_t")
  else()
    set(uintptr_t "uint32_t")
  endif()
endif(NOT HAVE_UINTPTR_T)
#
CHECK_TYPE_SIZE(wchar_t     SIZEOF_WCHAR_T)
if(HAVE_SIZEOF_WCHAR_T)
  set(HAVE_WCHAR_T 1)
endif(HAVE_SIZEOF_WCHAR_T)
#
# Check if _FILE_OFFSET_BITS macro needed for large files
#
CHECK_FILE_OFFSET_BITS()



#
# Check for Extended Attribute libraries, headers, and functions
#
if(ENABLE_XATTR)
  LA_CHECK_INCLUDE_FILE(attr/xattr.h     HAVE_ATTR_XATTR_H)
  LA_CHECK_INCLUDE_FILE(sys/xattr.h      HAVE_SYS_XATTR_H)
  CHECK_LIBRARY_EXISTS(attr "setxattr" "" HAVE_ATTR_LIB)
  if(HAVE_ATTR_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "attr")
  endif(HAVE_ATTR_LIB)
  CHECK_FUNCTION_EXISTS_GLIBC(extattr_get_file HAVE_EXTATTR_GET_FILE)
  CHECK_FUNCTION_EXISTS_GLIBC(extattr_list_file HAVE_EXTATTR_LIST_FILE)
  CHECK_FUNCTION_EXISTS_GLIBC(extattr_set_fd HAVE_EXTATTR_SET_FD)
  CHECK_FUNCTION_EXISTS_GLIBC(extattr_set_file HAVE_EXTATTR_SET_FILE)
  CHECK_FUNCTION_EXISTS_GLIBC(fsetxattr HAVE_FSETXATTR)
  CHECK_FUNCTION_EXISTS_GLIBC(getxattr HAVE_GETXATTR)
  CHECK_FUNCTION_EXISTS_GLIBC(lgetxattr HAVE_LGETXATTR)
  CHECK_FUNCTION_EXISTS_GLIBC(listxattr HAVE_LISTXATTR)
  CHECK_FUNCTION_EXISTS_GLIBC(llistxattr HAVE_LLISTXATTR)
  CHECK_FUNCTION_EXISTS_GLIBC(lsetxattr HAVE_LSETXATTR)
endif(ENABLE_XATTR)

#
# Check for ACL libraries, headers, and functions
#
# The ACL support in libarchive is written against the POSIX1e draft,
# which was never officially approved and varies quite a bit across
# platforms.  Worse, some systems have completely non-POSIX acl functions,
# which makes the following checks rather more complex than I would like.
#
if(ENABLE_ACL)
  CHECK_LIBRARY_EXISTS(acl "acl_get_file" "" HAVE_ACL_LIB)
  if(HAVE_ACL_LIB)
    set(CMAKE_REQUIRED_LIBRARIES "acl")
    find_library(ACL_LIBRARY NAMES acl)
    list(APPEND ADDITIONAL_LIBS ${ACL_LIBRARY})
  endif(HAVE_ACL_LIB)
  #
  CHECK_FUNCTION_EXISTS_GLIBC(acl_create_entry HAVE_ACL_CREATE_ENTRY)
  CHECK_FUNCTION_EXISTS_GLIBC(acl_init HAVE_ACL_INIT)
  CHECK_FUNCTION_EXISTS_GLIBC(acl_set_fd HAVE_ACL_SET_FD)
  CHECK_FUNCTION_EXISTS_GLIBC(acl_set_fd_np HAVE_ACL_SET_FD_NP)
  CHECK_FUNCTION_EXISTS_GLIBC(acl_set_file HAVE_ACL_SET_FILE)
  CHECK_TYPE_EXISTS(acl_permset_t "${INCLUDES}"    HAVE_ACL_PERMSET_T)

  # The "acl_get_perm()" function was omitted from the POSIX draft.
  # (It's a pretty obvious oversight; otherwise, there's no way to
  # test for specific permissions in a permset.)  Linux uses the obvious
  # name, FreeBSD adds _np to mark it as "non-Posix extension."
  # Test for both as a double-check that we really have POSIX-style ACL support.
  CHECK_SYMBOL_EXISTS(acl_get_perm "${INCLUDES}" HAVE_ACL_GET_PERM)
  CHECK_SYMBOL_EXISTS(acl_get_perm_np "${INCLUDES}" HAVE_ACL_GET_PERM_NP)
  CHECK_SYMBOL_EXISTS(acl_get_link "${INCLUDES}" HAVE_ACL_GET_LINK)
  CHECK_SYMBOL_EXISTS(acl_get_link_np "${INCLUDES}" HAVE_ACL_GET_LINK_NP)

  # MacOS has an acl.h that isn't POSIX.  It can be detected by
  # checking for ACL_USER
  CHECK_SYMBOL_EXISTS(ACL_USER "${INCLUDES}" HAVE_ACL_USER)
endif(ENABLE_ACL)

# Generate "config.h" from "build/cmake/config.h.in"
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DHAVE_CONFIG_H)

#
# Register installation of PDF documents.
#
if(WIN32 AND NOT CYGWIN)
  #
  # On Windows platform, It's better that we install PDF documents
  # on one's computer.
  # These PDF documents are available in the release package.
  #
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/pdf)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/pdf
            DESTINATION share/man
            FILES_MATCHING PATTERN "*.pdf"
    )
  endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/pdf)
endif(WIN32 AND NOT CYGWIN)
#
#
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libarchive)
#
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)
# Especially for early development, we want to be a little
# aggressive about diagnosing build problems; this can get
# relaxed somewhat in final shipping versions.
#if("CMAKE_C_COMPILER_ID" MATCHES "^GNU$")
#  add_definitions(-Wall -Werror)
#endif("CMAKE_C_COMPILER_ID" MATCHES "^GNU$")

if(ENABLE_TEST)
add_custom_target(run_all_tests)
endif(ENABLE_TEST)

add_subdirectory(libarchive)
#add_subdirectory(tar)
#add_subdirectory(cpio)

install(FILES COPYING DESTINATION ${CMake_DOC_DEST}/cmlibarchive)
