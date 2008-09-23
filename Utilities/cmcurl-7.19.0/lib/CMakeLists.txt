# Include all the necessary files for macros
set(CMAKE_MODULE_PATH "${CURL_SOURCE_DIR}/CMake")
include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckSymbolExists)
include (CheckCSourceCompiles)
# if crosscompiling is on, the CHECK_TYPE_SIZE macro coming with cmake uses
# TRY_COMPILE instead of TRY_RUN which makes crosscompiling easier, Alex
if(CMAKE_CROSSCOMPILING)  
  include ("${CMAKE_ROOT}/Modules/CheckTypeSize.cmake")
else(CMAKE_CROSSCOMPILING)
  include (CheckTypeSize)
endif(CMAKE_CROSSCOMPILING)


set(libCurl_SRCS 
  file.c timeval.c base64.c hostip.c progress.c formdata.c      
  cookie.c http.c sendf.c ftp.c url.c dict.c if2ip.c speedcheck.c
  ldap.c ssluse.c version.c getenv.c escape.c mprintf.c telnet.c
  netrc.c getinfo.c transfer.c strequal.c easy.c security.c krb4.c
  krb5.c memdebug.c http_chunks.c strtok.c connect.c llist.c hash.c
  multi.c content_encoding.c share.c http_digest.c md5.c
  http_negotiate.c http_ntlm.c inet_pton.c strtoofft.c strerror.c
  hostares.c hostasyn.c hostip4.c hostip6.c hostsyn.c hostthre.c
  inet_ntop.c parsedate.c select.c gtls.c sslgen.c tftp.c splay.c
  strdup.c socks.c ssh.c nss.c qssl.c
  arpa_telnet.h netrc.h file.h timeval.h qssl.h hostip.h        
  progress.h formdata.h cookie.h http.h sendf.h ftp.h url.h dict.h
  if2ip.h speedcheck.h urldata.h curl_ldap.h ssluse.h escape.h telnet.h
  getinfo.h strequal.h krb4.h memdebug.h inet_ntoa_r.h http_chunks.h
  strtok.h connect.h llist.h hash.h content_encoding.h share.h
  curl_md5.h http_digest.h http_negotiate.h http_ntlm.h inet_pton.h
  strtoofft.h strerror.h inet_ntop.h curlx.h memory.h setup.h   
  transfer.h select.h easyif.h multiif.h parsedate.h sslgen.h gtls.h
  tftp.h sockaddr.h splay.h strdup.h setup_once.h socks.h ssh.h nssg.h
  curl_base64.h
)

set(CURL_DISABLE_LDAP 1)
if(NOT CURL_DISABLE_LDAP)
  set(libCurl_SRCS
    ${libCurl_SRCS}
    ldap.c
    )
endif(NOT CURL_DISABLE_LDAP)

# if we have Kerberos 4, right now this is never on
#OPTION(CURL_KRB4 "Use Kerberos 4" OFF)
if(CURL_KRB4)
  set(libCurl_SRCS ${libCurl_SRCS}
    krb4.c
    security.c
    )
endif(CURL_KRB4)

#OPTION(CURL_MALLOC_DEBUG "Debug mallocs in Curl" OFF)
mark_as_advanced(CURL_MALLOC_DEBUG)
if(CURL_MALLOC_DEBUG)
  set(libCurl_SRCS ${libCurl_SRCS}
    memdebug.c
    )
endif(CURL_MALLOC_DEBUG)

# On windows preload settings
if(WIN32)
  include(${CURL_SOURCE_DIR}/CMake/WindowsCache.cmake)
endif(WIN32)

# This macro checks if the symbol exists in the library and if it
# does, it appends library to the list.
set(CURL_LIBS "")
macro(CHECK_LIBRARY_EXISTS_CONCAT LIBRARY SYMBOL VARIABLE)
  check_library_exists("${LIBRARY};${CURL_LIBS}" ${SYMBOL} "" 
    ${VARIABLE})
  if(${VARIABLE})
    set(CURL_LIBS ${CURL_LIBS} ${LIBRARY})
  endif(${VARIABLE})
endmacro(CHECK_LIBRARY_EXISTS_CONCAT)

# Check for all needed libraries
check_library_exists_concat("dl"     dlopen       HAVE_LIBDL)
#CHECK_LIBRARY_EXISTS_CONCAT("ucb"    gethostname  HAVE_LIBUCB)
check_library_exists_concat("socket" connect      HAVE_LIBSOCKET)
check_library_exists("c" gethostbyname "" NOT_NEED_LIBNSL)

# Yellowtab Zeta needs different libraries than BeOS 5.
if(BEOS)
  set(NOT_NEED_LIBNSL 1)
  check_library_exists_concat("bind" gethostbyname HAVE_LIBBIND)
  check_library_exists_concat("bnetapi" closesocket HAVE_LIBBNETAPI)
endif(BEOS)

check_library_exists_concat("network" recv HAVE_LIBNETWORK)

if(NOT NOT_NEED_LIBNSL)
  check_library_exists_concat("nsl"    gethostbyname  HAVE_LIBNSL)
endif(NOT NOT_NEED_LIBNSL)

check_library_exists_concat("ws2_32" getch        HAVE_LIBWS2_32)
check_library_exists_concat("winmm"  getch        HAVE_LIBWINMM)
if(NOT CURL_SPECIAL_LIBZ)
  check_library_exists_concat("z"      inflateEnd   HAVE_LIBZ)
endif(NOT CURL_SPECIAL_LIBZ)

option(CMAKE_USE_OPENSSL "Use OpenSSL code. Experimental" ON)
mark_as_advanced(CMAKE_USE_OPENSSL)
if(CMAKE_USE_OPENSSL)
  check_library_exists_concat("crypto" CRYPTO_lock  HAVE_LIBCRYPTO)
  check_library_exists_concat("ssl"    SSL_connect  HAVE_LIBSSL)
endif(CMAKE_USE_OPENSSL)

# Check for idn
check_library_exists_concat("idn" idna_to_ascii_lz HAVE_LIBIDN)

# Check for symbol dlopen (same as HAVE_LIBDL)
check_library_exists("${CURL_LIBS}" dlopen "" HAVE_DLOPEN)

# For other tests to use the same libraries
set(CMAKE_REQUIRED_LIBRARIES ${CURL_LIBS})

if(CURL_SPECIAL_LIBZ)
  set(CURL_LIBS ${CURL_LIBS} "${CURL_SPECIAL_LIBZ}")
  include_directories(${CURL_SPECIAL_LIBZ_INCLUDES})
  set(HAVE_LIBZ 0)
  set(HAVE_ZLIB_H 0)
endif(CURL_SPECIAL_LIBZ)

# do we have process.h
check_include_file("process.h" HAVE_PROCESS_H)

# Check if header file exists and add it to the list.
macro(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  check_include_files("${CURL_INCLUDES};${FILE}" ${VARIABLE})
  if(${VARIABLE})
    set(CURL_INCLUDES ${CURL_INCLUDES} ${FILE})
  endif(${VARIABLE})
endmacro(CHECK_INCLUDE_FILE_CONCAT)

# Check for header files
if(NOT UNIX)
  check_include_file_concat("ws2tcpip.h"     HAVE_WS2TCPIP_H)
  check_include_file_concat("winsock2.h"     HAVE_WINSOCK2_H)
endif(NOT UNIX)
check_include_file_concat("stdio.h"          HAVE_STDIO_H)
if(NOT UNIX)
  check_include_file_concat("windows.h"      HAVE_WINDOWS_H)
  check_include_file_concat("winsock.h"      HAVE_WINSOCK_H)
endif(NOT UNIX)
check_include_file_concat("stddef.h"         HAVE_STDDEF_H)
check_include_file_concat("sys/types.h"      HAVE_SYS_TYPES_H)
check_include_file_concat("inttypes.h"       HAVE_INTTYPES_H)
check_include_file_concat("alloca.h"         HAVE_ALLOCA_H)
check_include_file_concat("arpa/inet.h"      HAVE_ARPA_INET_H)
check_include_file_concat("dlfcn.h"          HAVE_DLFCN_H)
check_include_file_concat("fcntl.h"          HAVE_FCNTL_H)
check_include_file_concat("malloc.h"         HAVE_MALLOC_H)
check_include_file_concat("memory.h"         HAVE_MEMORY_H)
check_include_file_concat("netdb.h"          HAVE_NETDB_H)
check_include_file_concat("sys/poll.h"       HAVE_SYS_POLL_H)
check_include_file_concat("assert.h"         HAVE_ASSERT_H)
check_include_file_concat("limits.h"         HAVE_LIMITS_H)

if(CMAKE_USE_OPENSSL)
  check_include_file_concat("openssl/x509.h"   HAVE_OPENSSL_X509_H)
  check_include_file_concat("openssl/engine.h" HAVE_OPENSSL_ENGINE_H)
  check_include_file_concat("openssl/rsa.h"    HAVE_OPENSSL_RSA_H)
  check_include_file_concat("openssl/crypto.h" HAVE_OPENSSL_CRYPTO_H)
  check_include_file_concat("openssl/pem.h"    HAVE_OPENSSL_PEM_H)
  check_include_file_concat("openssl/ssl.h"    HAVE_OPENSSL_SSL_H)
  check_include_file_concat("openssl/err.h"    HAVE_OPENSSL_ERR_H)
  check_include_file_concat("openssl/rand.h"   HAVE_OPENSSL_RAND_H)
  check_include_file_concat("openssl/pkcs12.h" HAVE_OPENSSL_PKCS12_H)
endif(CMAKE_USE_OPENSSL)

if(NOT CURL_SPECIAL_LIBZ)
  check_include_file_concat("zlib.h"           HAVE_ZLIB_H)
endif(NOT CURL_SPECIAL_LIBZ)
check_include_file_concat("sys/socket.h"     HAVE_SYS_SOCKET_H)
check_include_file_concat("netinet/in.h"     HAVE_NETINET_IN_H)
check_include_file_concat("net/if.h"         HAVE_NET_IF_H)
check_include_file_concat("netinet/if_ether.h" 
  HAVE_NETINET_IF_ETHER_H)
check_include_file_concat("netinet/tcp.h" 
  HAVE_NETINET_TCP_H)
check_include_file_concat("sys/select.h"    HAVE_SYS_SELECT_H)
check_include_file_concat("utime.h"         HAVE_UTIME_H)
check_include_file_concat("netinet/in.h"    HAVE_NETINET_IN_H)
check_include_file_concat("pwd.h"           HAVE_PWD_H)
check_include_file_concat("sgtty.h"         HAVE_SGTTY_H)
check_include_file_concat("stdint.h"        HAVE_STDINT_H)
check_include_file_concat("stdlib.h"        HAVE_STDLIB_H)
check_include_file_concat("string.h"        HAVE_STRING_H)
check_include_file_concat("strings.h"       HAVE_STRINGS_H)
check_include_file_concat("sys/param.h"     HAVE_SYS_PARAM_H)
check_include_file_concat("sys/stat.h"      HAVE_SYS_STAT_H)
check_include_file_concat("sys/time.h"      HAVE_SYS_TIME_H)
check_include_file_concat("sys/resource.h"  HAVE_SYS_RESOURCE_H)
check_include_file_concat("termios.h"       HAVE_TERMIOS_H)
check_include_file_concat("termio.h"        HAVE_TERMIO_H)
check_include_file_concat("io.h"            HAVE_IO_H)
check_include_file_concat("time.h"          HAVE_TIME_H)
check_include_file_concat("unistd.h"        HAVE_UNISTD_H)
check_include_file_concat("sys/utime.h"     HAVE_SYS_UTIME_H)
check_include_file_concat("sockio.h"        HAVE_SOCKIO_H)
check_include_file_concat("sys/sockio.h"    HAVE_SYS_SOCKIO_H)
check_include_file_concat("x509.h"          HAVE_X509_H)
check_include_file_concat("locale.h"        HAVE_LOCALE_H)
check_include_file_concat("setjmp.h"        HAVE_SETJMP_H)
check_include_file_concat("signal.h"        HAVE_SIGNAL_H)
check_include_file_concat("sys/ioctl.h"     HAVE_SYS_IOCTL_H)
check_include_file_concat("sys/utsname.h"   HAVE_SYS_UTSNAME_H)
check_include_file_concat("idn-free.h"      HAVE_IDN_FREE_H)
check_include_file_concat("idna.h"          HAVE_IDNA_H)
check_include_file_concat("tld.h"           HAVE_TLD_H)
check_include_file_concat("arpa/tftp.h"     HAVE_ARPA_TFTP_H)
check_include_file_concat("errno.h"         HAVE_ERRNO_H)
check_include_file_concat("libgen.h"        HAVE_LIBGEN_H)
check_include_file_concat("sys/filio.h"     HAVE_SYS_FILIO_H)
check_type_size(size_t  SIZEOF_SIZE_T)
check_type_size(ssize_t  SIZEOF_SSIZE_T)
check_type_size("long long"  SIZEOF_LONG_LONG)
check_type_size("long"  SIZEOF_LONG)
check_type_size("__int64"  SIZEOF___INT64)
check_type_size("long double"  SIZEOF_LONG_DOUBLE)
check_type_size("time_t"  SIZEOF_TIME_T)
if(NOT HAVE_SIZEOF_SSIZE_T)
  if(SIZEOF_LONG EQUAL SIZEOF_SIZE_T)
    set(ssize_t long)
  endif(SIZEOF_LONG EQUAL SIZEOF_SIZE_T)
  if(NOT ssize_t AND SIZEOF___INT64 EQUAL SIZEOF_SIZE_T)
    set(ssize_t __int64)
  endif(NOT ssize_t AND SIZEOF___INT64 EQUAL SIZEOF_SIZE_T)
endif(NOT HAVE_SIZEOF_SSIZE_T)

if(HAVE_SIZEOF_LONG_LONG)
  set(HAVE_LONGLONG 1)
  set(HAVE_LL 1)
endif(HAVE_SIZEOF_LONG_LONG)

find_file(RANDOM_FILE urandom /dev)
mark_as_advanced(RANDOM_FILE)

# Check for some functions that are used
check_symbol_exists(basename      "${CURL_INCLUDES}" HAVE_BASENAME)
check_symbol_exists(socket        "${CURL_INCLUDES}" HAVE_SOCKET)
check_symbol_exists(poll          "${CURL_INCLUDES}" HAVE_POLL)
check_symbol_exists(select        "${CURL_INCLUDES}" HAVE_SELECT)
check_symbol_exists(strdup        "${CURL_INCLUDES}" HAVE_STRDUP)
check_symbol_exists(strstr        "${CURL_INCLUDES}" HAVE_STRSTR)
check_symbol_exists(strtok_r      "${CURL_INCLUDES}" HAVE_STRTOK_R)
check_symbol_exists(strftime      "${CURL_INCLUDES}" HAVE_STRFTIME)
check_symbol_exists(uname         "${CURL_INCLUDES}" HAVE_UNAME)
check_symbol_exists(strcasecmp    "${CURL_INCLUDES}" HAVE_STRCASECMP)
check_symbol_exists(stricmp       "${CURL_INCLUDES}" HAVE_STRICMP)
check_symbol_exists(strcmpi       "${CURL_INCLUDES}" HAVE_STRCMPI)
check_symbol_exists(strncmpi      "${CURL_INCLUDES}" HAVE_STRNCMPI)
check_symbol_exists(basename      "${CURL_INCLUDES}" HAVE_BASENAME)
if(NOT HAVE_STRNCMPI)
  set(HAVE_STRCMPI)
endif(NOT HAVE_STRNCMPI)
check_symbol_exists(gethostbyaddr "${CURL_INCLUDES}" HAVE_GETHOSTBYADDR)
check_symbol_exists(gettimeofday  "${CURL_INCLUDES}" HAVE_GETTIMEOFDAY)
check_symbol_exists(inet_addr     "${CURL_INCLUDES}" HAVE_INET_ADDR)
check_symbol_exists(inet_pton     "${CURL_INCLUDES}" HAVE_INET_PTON)
check_symbol_exists(inet_ntoa     "${CURL_INCLUDES}" HAVE_INET_NTOA)
check_symbol_exists(inet_ntoa_r   "${CURL_INCLUDES}" HAVE_INET_NTOA_R)
check_symbol_exists(tcsetattr     "${CURL_INCLUDES}" HAVE_TCSETATTR)
check_symbol_exists(tcgetattr     "${CURL_INCLUDES}" HAVE_TCGETATTR)
check_symbol_exists(perror        "${CURL_INCLUDES}" HAVE_PERROR)
check_symbol_exists(closesocket   "${CURL_INCLUDES}" HAVE_CLOSESOCKET)
check_symbol_exists(setvbuf       "${CURL_INCLUDES}" HAVE_SETVBUF)
check_symbol_exists(sigsetjmp     "${CURL_INCLUDES}" HAVE_SIGSETJMP)
check_symbol_exists(getpass_r     "${CURL_INCLUDES}" HAVE_GETPASS_R)
check_symbol_exists(strlcat       "${CURL_INCLUDES}" HAVE_STRLCAT)
check_symbol_exists(getpwuid      "${CURL_INCLUDES}" HAVE_GETPWUID)
check_symbol_exists(geteuid       "${CURL_INCLUDES}" HAVE_GETEUID)
check_symbol_exists(utime         "${CURL_INCLUDES}" HAVE_UTIME)
if(CMAKE_USE_OPENSSL)
  check_symbol_exists(RAND_status   "${CURL_INCLUDES}" HAVE_RAND_STATUS)
  check_symbol_exists(RAND_screen   "${CURL_INCLUDES}" HAVE_RAND_SCREEN)
  check_symbol_exists(RAND_egd      "${CURL_INCLUDES}" HAVE_RAND_EGD)
  check_symbol_exists(CRYPTO_cleanup_all_ex_data "${CURL_INCLUDES}" 
    HAVE_CRYPTO_CLEANUP_ALL_EX_DATA)
endif(CMAKE_USE_OPENSSL)
check_symbol_exists(gmtime_r      "${CURL_INCLUDES}" HAVE_GMTIME_R)
check_symbol_exists(localtime_r   "${CURL_INCLUDES}" HAVE_LOCALTIME_R)

check_symbol_exists(gethostbyname   "${CURL_INCLUDES}" HAVE_GETHOSTBYNAME)
check_symbol_exists(gethostbyname_r "${CURL_INCLUDES}" HAVE_GETHOSTBYNAME_R)
check_symbol_exists(gethostbyaddr_r "${CURL_INCLUDES}" HAVE_GETHOSTBYADDR_R)

check_symbol_exists(signal        "${CURL_INCLUDES}" HAVE_SIGNAL_FUNC)
check_symbol_exists(SIGALRM       "${CURL_INCLUDES}" HAVE_SIGNAL_MACRO)
if(HAVE_SIGNAL_FUNC AND HAVE_SIGNAL_MACRO)
  set(HAVE_SIGNAL 1)
endif(HAVE_SIGNAL_FUNC AND HAVE_SIGNAL_MACRO)
check_symbol_exists(uname          "${CURL_INCLUDES}" HAVE_UNAME)
check_symbol_exists(strtoll        "${CURL_INCLUDES}" HAVE_STRTOLL)
check_symbol_exists(_strtoi64      "${CURL_INCLUDES}" HAVE__STRTOI64)
check_symbol_exists(strerror_r     "${CURL_INCLUDES}" HAVE_STRERROR_R)
check_symbol_exists(siginterrupt   "${CURL_INCLUDES}" HAVE_SIGINTERRUPT)
check_symbol_exists(perror         "${CURL_INCLUDES}" HAVE_PERROR)
check_symbol_exists(fork           "${CURL_INCLUDES}" HAVE_FORK)
check_symbol_exists(pipe           "${CURL_INCLUDES}" HAVE_PIPE)
check_symbol_exists(ftruncate      "${CURL_INCLUDES}" HAVE_FTRUNCATE)
check_symbol_exists(getprotobyname "${CURL_INCLUDES}" HAVE_GETPROTOBYNAME)
check_symbol_exists(getrlimit      "${CURL_INCLUDES}" HAVE_GETRLIMIT)
check_symbol_exists(idn_free       "${CURL_INCLUDES}" HAVE_IDN_FREE)
check_symbol_exists(idna_strerror  "${CURL_INCLUDES}" HAVE_IDNA_STRERROR)
check_symbol_exists(tld_strerror   "${CURL_INCLUDES}" HAVE_TLD_STRERROR)
check_symbol_exists(setlocale      "${CURL_INCLUDES}" HAVE_SETLOCALE)
check_symbol_exists(setrlimit      "${CURL_INCLUDES}" HAVE_SETRLIMIT)


# sigaction and sigsetjmp are special. Use special mechanism for
# detecting those, but only if previous attempt failed.
if(HAVE_SIGNAL_H)
  check_symbol_exists(sigaction "signal.h" HAVE_SIGACTION)
endif(HAVE_SIGNAL_H)

if(NOT HAVE_SIGSETJMP)
  if(HAVE_SETJMP_H)
    check_symbol_exists(sigsetjmp "setjmp.h" HAVE_MACRO_SIGSETJMP)
    if(HAVE_MACRO_SIGSETJMP)
      set(HAVE_SIGSETJMP 1)
    endif(HAVE_MACRO_SIGSETJMP)
  endif(HAVE_SETJMP_H)
endif(NOT HAVE_SIGSETJMP)

# For other curl specific tests, use this macro.
macro(CURL_INTERNAL_TEST CURL_TEST)
  if("${CURL_TEST}" MATCHES "^${CURL_TEST}$")
    set(MACRO_CHECK_FUNCTION_DEFINITIONS 
      "-D${CURL_TEST} ${CMAKE_REQUIRED_FLAGS}")
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CURL_TEST_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    endif(CMAKE_REQUIRED_LIBRARIES)

    message(STATUS "Performing Curl Test ${CURL_TEST}")
    try_compile(${CURL_TEST}
      ${CMAKE_BINARY_DIR}
      ${CURL_SOURCE_DIR}/CMake/CurlTests.c
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      "${CURL_TEST_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    if(${CURL_TEST})
      set(${CURL_TEST} 1 CACHE INTERNAL "Curl test ${FUNCTION}")
      message(STATUS "Performing Curl Test ${CURL_TEST} - Success")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Performing Curl Test ${CURL_TEST} passed with the following output:\n"
        "${OUTPUT}\n")
    else(${CURL_TEST})
      message(STATUS "Performing Curl Test ${CURL_TEST} - Failed")
      set(${CURL_TEST} "" CACHE INTERNAL "Curl test ${FUNCTION}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Performing Curl Test ${CURL_TEST} failed with the following output:\n"
        "${OUTPUT}\n")
    endif(${CURL_TEST})
  endif("${CURL_TEST}" MATCHES "^${CURL_TEST}$")
endmacro(CURL_INTERNAL_TEST) 

macro(CURL_INTERNAL_TEST_RUN CURL_TEST)
  if("${CURL_TEST}_COMPILE" MATCHES "^${CURL_TEST}_COMPILE$")
    set(MACRO_CHECK_FUNCTION_DEFINITIONS 
      "-D${CURL_TEST} ${CMAKE_REQUIRED_FLAGS}")
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CURL_TEST_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    endif(CMAKE_REQUIRED_LIBRARIES)

    message(STATUS "Performing Curl Test ${CURL_TEST}")
    try_run(${CURL_TEST} ${CURL_TEST}_COMPILE
      ${CMAKE_BINARY_DIR}
      ${CURL_SOURCE_DIR}/CMake/CurlTests.c
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      "${CURL_TEST_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    if(${CURL_TEST}_COMPILE AND NOT ${CURL_TEST})
      set(${CURL_TEST} 1 CACHE INTERNAL "Curl test ${FUNCTION}")
      message(STATUS "Performing Curl Test ${CURL_TEST} - Success")
    else(${CURL_TEST}_COMPILE AND NOT ${CURL_TEST})
      message(STATUS "Performing Curl Test ${CURL_TEST} - Failed")
      set(${CURL_TEST} "" CACHE INTERNAL "Curl test ${FUNCTION}")
      file(APPEND "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log"
        "Performing Curl Test ${CURL_TEST} failed with the following output:\n"
        "${OUTPUT}")
      if(${CURL_TEST}_COMPILE)
        file(APPEND 
          "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log" 
          "There was a running problem of this test\n")
      endif(${CURL_TEST}_COMPILE)
      file(APPEND "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log" 
        "\n\n")
    endif(${CURL_TEST}_COMPILE AND NOT ${CURL_TEST})
  endif("${CURL_TEST}_COMPILE" MATCHES "^${CURL_TEST}_COMPILE$")
endmacro(CURL_INTERNAL_TEST_RUN) 

# Do curl specific tests
#OPTION(CURL_HAVE_DISABLED_NONBLOCKING "Disable non-blocking socket detection" OFF)
set(CURL_NONBLOCKING_TESTS)
if(NOT CURL_HAVE_DISABLED_NONBLOCKING)
  set(CURL_NONBLOCKING_TESTS
    HAVE_FIONBIO
    HAVE_IOCTLSOCKET
    HAVE_IOCTLSOCKET_CASE
    HAVE_O_NONBLOCK
    HAVE_SO_NONBLOCK
    )
endif(NOT CURL_HAVE_DISABLED_NONBLOCKING)
foreach(CURL_TEST 
    ${CURL_NONBLOCKING_TESTS}
    TIME_WITH_SYS_TIME
    HAVE_O_NONBLOCKHAVE_GETHOSTBYADDR_R_5
    HAVE_GETHOSTBYADDR_R_7
    HAVE_GETHOSTBYADDR_R_8
    HAVE_GETHOSTBYADDR_R_5_REENTRANT
    HAVE_GETHOSTBYADDR_R_7_REENTRANT
    HAVE_GETHOSTBYADDR_R_8_REENTRANT
    HAVE_GETHOSTBYNAME_R_3
    HAVE_GETHOSTBYNAME_R_5
    HAVE_GETHOSTBYNAME_R_6
    HAVE_GETHOSTBYNAME_R_3_REENTRANT
    HAVE_GETHOSTBYNAME_R_5_REENTRANT
    HAVE_GETHOSTBYNAME_R_6_REENTRANT
    HAVE_SOCKLEN_T
    HAVE_IN_ADDR_T
    STDC_HEADERS
    RETSIGTYPE_TEST
    HAVE_INET_NTOA_R_DECL
    HAVE_INET_NTOA_R_DECL_REENTRANT
    HAVE_GETADDRINFO
    HAVE_FILE_OFFSET_BITS
    )
  curl_internal_test(${CURL_TEST})
endforeach(CURL_TEST)
if(HAVE_FILE_OFFSET_BITS)
  set(_FILE_OFFSET_BITS 64)
endif(HAVE_FILE_OFFSET_BITS)

foreach(CURL_TEST 
    HAVE_GLIBC_STRERROR_R
    HAVE_POSIX_STRERROR_R
    )
  curl_internal_test_run(${CURL_TEST})
endforeach(CURL_TEST)

# Check for reentrant
foreach(CURL_TEST
    HAVE_GETHOSTBYADDR_R_5
    HAVE_GETHOSTBYADDR_R_7
    HAVE_GETHOSTBYADDR_R_8
    HAVE_GETHOSTBYNAME_R_3
    HAVE_GETHOSTBYNAME_R_5
    HAVE_GETHOSTBYNAME_R_6
    HAVE_INET_NTOA_R_DECL_REENTRANT)
  if(NOT ${CURL_TEST})
    if(${CURL_TEST}_REENTRANT)
      set(NEED_REENTRANT 1)
    endif(${CURL_TEST}_REENTRANT)
  endif(NOT ${CURL_TEST})
endforeach(CURL_TEST)

if(NEED_REENTRANT)
  foreach(CURL_TEST
      HAVE_GETHOSTBYADDR_R_5
      HAVE_GETHOSTBYADDR_R_7
      HAVE_GETHOSTBYADDR_R_8
      HAVE_GETHOSTBYNAME_R_3
      HAVE_GETHOSTBYNAME_R_5
      HAVE_GETHOSTBYNAME_R_6)
    set(${CURL_TEST} 0)
    if(${CURL_TEST}_REENTRANT)
      set(${CURL_TEST} 1)
    endif(${CURL_TEST}_REENTRANT)
  endforeach(CURL_TEST)
endif(NEED_REENTRANT)

if(HAVE_INET_NTOA_R_DECL_REENTRANT)
  set(HAVE_INET_NTOA_R_DECL 1)
  set(NEED_REENTRANT 1)
endif(HAVE_INET_NTOA_R_DECL_REENTRANT)

# Some other minor tests

if(NOT HAVE_SOCKLEN_T)
  set(socklen_t "int")
endif(NOT HAVE_SOCKLEN_T)

if(NOT HAVE_IN_ADDR_T)
  set(in_addr_t "unsigned long")
endif(NOT HAVE_IN_ADDR_T)

# Fix libz / zlib.h

if(NOT CURL_SPECIAL_LIBZ)
  if(NOT HAVE_LIBZ)
    set(HAVE_ZLIB_H 0)
  endif(NOT HAVE_LIBZ)

  if(NOT HAVE_ZLIB_H)
    set(HAVE_LIBZ 0)
  endif(NOT HAVE_ZLIB_H)
endif(NOT CURL_SPECIAL_LIBZ)

if(_FILE_OFFSET_BITS)
  set(_FILE_OFFSET_BITS 64)
endif(_FILE_OFFSET_BITS)
set(CMAKE_REQUIRED_FLAGS "-D_FILE_OFFSET_BITS=64")
set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/curl/curl.h")
check_type_size("curl_off_t" SIZEOF_CURL_OFF_T)
set(CMAKE_EXTRA_INCLUDE_FILES)
set(CMAKE_REQUIRED_FLAGS)


# Check for nonblocking
set(HAVE_DISABLED_NONBLOCKING 1)
if(HAVE_FIONBIO OR 
    HAVE_IOCTLSOCKET OR
    HAVE_IOCTLSOCKET_CASE OR
    HAVE_O_NONBLOCK)
  set(HAVE_DISABLED_NONBLOCKING)
endif(HAVE_FIONBIO OR 
  HAVE_IOCTLSOCKET OR
  HAVE_IOCTLSOCKET_CASE OR
  HAVE_O_NONBLOCK)

if(RETSIGTYPE_TEST)
  set(RETSIGTYPE void)
else(RETSIGTYPE_TEST)
  set(RETSIGTYPE int)
endif(RETSIGTYPE_TEST)

if(CMAKE_COMPILER_IS_GNUCC AND APPLE)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-Wno-long-double HAVE_C_FLAG_Wno_long_double)
  if(HAVE_C_FLAG_Wno_long_double)
    # The Mac version of GCC warns about use of long double.  Disable it.
    get_source_file_property(MPRINTF_COMPILE_FLAGS mprintf.c COMPILE_FLAGS)
    if(MPRINTF_COMPILE_FLAGS)
      set(MPRINTF_COMPILE_FLAGS "${MPRINTF_COMPILE_FLAGS} -Wno-long-double")
    else(MPRINTF_COMPILE_FLAGS)
      set(MPRINTF_COMPILE_FLAGS "-Wno-long-double")
    endif(MPRINTF_COMPILE_FLAGS)
    set_source_files_properties(mprintf.c PROPERTIES
      COMPILE_FLAGS ${MPRINTF_COMPILE_FLAGS})
  endif(HAVE_C_FLAG_Wno_long_double)
endif(CMAKE_COMPILER_IS_GNUCC AND APPLE)
include(${CURL_SOURCE_DIR}/CMake/OtherTests.cmake)


# The rest of the build

include_directories(${CURL_SOURCE_DIR})
include_directories(${CURL_BINARY_DIR})
if(WIN32)
  set(WIN32_LEAN_AND_MEAN TRUE)
endif(WIN32)

configure_file(${CURL_SOURCE_DIR}/lib/config.cmake.h.in
  ${CURL_BINARY_DIR}/lib/config.h)
set(CURL_SIZEOF_LONG ${SIZEOF_LONG})
configure_file(${CURL_SOURCE_DIR}/include/curl/curlbuild.cmake.h.in
  ${CURL_BINARY_DIR}/include/curl/configbuild.h)

add_library(cmcurl ${libCurl_SRCS})
target_link_libraries(cmcurl ${CURL_LIBS})

option(CURL_TESTING "Do libCurl testing" OFF)
if(CURL_TESTING)
  subdirs(Testing)
endif(CURL_TESTING)

#add_executable(LIBCURL Testing/curltest.c)
#target_link_libraries(LIBCURL cmcurl ${CMAKE_DL_LIBS})
#add_test(curl "${EXECUTABLE_OUTPUT_PATH}/LIBCURL")
