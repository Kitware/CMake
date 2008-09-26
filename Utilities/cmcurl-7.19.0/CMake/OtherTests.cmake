include(CurlCheckCSourceCompiles)
set(EXTRA_DEFINES "__unused1\n#undef inline\n#define __unused2")
set(HEADER_INCLUDES)
set(headers_hack)

macro(add_header_include check header)
  if(${check})
    set(headers_hack
      "${headers_hack}\n#include <${header}>")
    #SET(HEADER_INCLUDES
    #  ${HEADER_INCLUDES}
    #  "${header}")
  endif(${check})
endmacro(add_header_include)

set(signature_call_conv)
if(HAVE_WINDOWS_H)
  add_header_include(HAVE_WINDOWS_H "windows.h")
  add_header_include(HAVE_WINSOCK2_H "winsock2.h")
  add_header_include(HAVE_WINSOCK_H "winsock.h")
  set(EXTRA_DEFINES ${EXTRA_DEFINES}
    "__unused7\n#ifndef WIN32_LEAN_AND_MEAN\n#define WIN32_LEAN_AND_MEAN\n#endif\n#define __unused3")
  set(signature_call_conv "PASCAL")
else(HAVE_WINDOWS_H)
  add_header_include(HAVE_SYS_TYPES_H "sys/types.h")
  add_header_include(HAVE_SYS_SOCKET_H "sys/socket.h")
  add_header_include(HAVE_SYS_TIME_H "sys/time.h")
endif(HAVE_WINDOWS_H)

set(EXTRA_DEFINES_BACKUP "${EXTRA_DEFINES}")
set(EXTRA_DEFINES "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}\n#define __unused5")
curl_check_c_source_compiles(
" struct timeval ts;
  ts.tv_sec  = 0;
  ts.tv_usec = 0;"
HAVE_STRUCT_TIMEVAL)


# function to find and set curl_typeof_curl_off_t
# trytypes should be the name of a variable that 
# has the list of types to try, and size is the size in bytes
# we are trying to find
function(curl_find_curl_off_t trytypes size)
  if(DEFINED curl_typeof_curl_off_t)
    return()
  endif(DEFINED curl_typeof_curl_off_t)
  foreach(type ${${trytypes}}) 
    # force the try compile to try until it works
    set(curl_typeof_curl_off_t "UNKNOWN")
    set(EXTRA_DEFINES 
      "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}\n#define __unused5")
    curl_check_c_source_compiles(
      "
        typedef ${type} curl_off_t;
        typedef char dummy_arr[sizeof(curl_off_t) == ${size} ? 1 : -1];
        curl_off_t dummy;
       "
      curl_typeof_curl_off_t
      )
    if(curl_typeof_curl_off_t)
      # this means it found the type and we can return
      set(CURL_TYPEOF_CURL_OFF_T "${type}" CACHE INTERNAL "type of curl_off_t")
      return()
    endif(curl_typeof_curl_off_t)
  endforeach(type)
endfunction(curl_find_curl_off_t)


if(NOT DEFINED curl_typeof_curl_off_t)
  set(curl_show_typeof_status 1)
endif(NOT DEFINED curl_typeof_curl_off_t)
# check for an 8 byte off_t type
set(try_types_8 "long" "__int64" "long long"
    "int64_t" "__longlong" "__longlog_t")
curl_find_curl_off_t(try_types_8 8 )
# check for a 4 byte off_t type
set(try_types_4 "long" "int32_t" "__int32" "int")
curl_find_curl_off_t(try_types_4 4 )
set(try_types_2 "long" "ubt16_t" "__int16" "int")
curl_find_curl_off_t(try_types_2 2 )
if(curl_show_typeof_status)
  message(STATUS "curl_typeof_curl_off_t = ${curl_typeof_curl_off_t}") 
  check_type_size("${CURL_TYPEOF_CURL_OFF_T}" CURL_SIZEOF_CURL_OFF_T)
  message(STATUS "sizeof ${curl_typeof_curl_off_t} = ${CURL_SIZEOF_CURL_OFF_T}") 
endif(curl_show_typeof_status)


# need to set the format strings used in printf for off_t
# curl_format_curl_off_t   - signed 
# curl_format_curl_off_tu  - unsigned
# 
# First see if the type is 2 4 or 8 bytes
if(CURL_SIZEOF_CURL_OFF_T EQUAL 2)
  set(PRI_MACRO "PRId16")
endif(CURL_SIZEOF_CURL_OFF_T EQUAL 2)
if(CURL_SIZEOF_CURL_OFF_T EQUAL 4)
  set(PRI_MACRO "PRId32")
endif(CURL_SIZEOF_CURL_OFF_T EQUAL 4)
if(CURL_SIZEOF_CURL_OFF_T EQUAL 8)
  set(PRI_MACRO "PRId64")
endif(CURL_SIZEOF_CURL_OFF_T EQUAL 8)
# now see if PRI macros are defined for printing
set(EXTRA_DEFINES "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}\n#define __unused5")
curl_check_c_source_compiles("char f[] = ${PRI_MACRO};" curl_pri_macro)
if(curl_pri_macro)
  string(REPLACE i u "${PRI_MACRO}" PRI_MACROU)
  string(REPLACE d u "${PRI_MACRO}" PRI_MACROU)
  string(REPLACE D U "${PRI_MACRO}" PRI_MACROU)
  set(CURL_FORMAT_CURL_OFF_T "${PRI_MACRO}" CACHE INTERNAL "print curl_off_t format string")
  set(CURL_FORMAT_CURL_OFF_TU "${PRI_MACROU}" CACHE INTERNAL "print curl_off_t format string")
else(curl_pri_macro)
  if(CURL_TYPEOF_CURL_OFF_T MATCHES "long.*long")
    set(CURL_FORMAT_CURL_OFF_T  "lld" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "llu" CACHE INTERNAL "print curl_off_t format string")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "long")
    set(CURL_FORMAT_CURL_OFF_T  "ld" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "lu" CACHE INTERNAL "print curl_off_t format string")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "int")
    set(CURL_FORMAT_CURL_OFF_T  "d" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "u" CACHE INTERNAL "print curl_off_t format string")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int64")
    set(CURL_FORMAT_CURL_OFF_T  "I64d" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "I64u" CACHE INTERNAL "print curl_off_t format string")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int32")
    set(CURL_FORMAT_CURL_OFF_T  "I32d" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "I32u" CACHE INTERNAL "print curl_off_t format string")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int16")
    set(CURL_FORMAT_CURL_OFF_T  "I16d" CACHE INTERNAL "print curl_off_t format string")
    set(CURL_FORMAT_CURL_OFF_TU "I16u" CACHE INTERNAL "print curl_off_t format string")
  endif()
  message(STATUS "CURL_FORMAT_CURL_OFF_T = ${CURL_FORMAT_CURL_OFF_T}")
  message(STATUS "CURL_FORMAT_CURL_OFF_TU = ${CURL_FORMAT_CURL_OFF_TU}")
endif(curl_pri_macro)

function (check_curl_off_t_suffix )
  if(DEFINED CURL_SUFFIX_CURL_OFF_T)
    return()
  endif()
  set(CURL_TYPEOF_CURL_OFF_T "long long")
  if(CURL_TYPEOF_CURL_OFF_T MATCHES "long.*long")
    set(curl_test_suffix "LL")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "long")
    set(curl_test_suffix "L")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "int")
    set(curl_test_suffix "")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int64")
    set(curl_test_suffix "LL" "i64")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int32")
    set(curl_test_suffix "L" "i32")
  endif()
  if(CURL_TYPEOF_CURL_OFF_T STREQUAL "__int16")
    set(curl_test_suffix "L" "i16")
  endif()
  foreach (suffix ${curl_test_suffix})
    set(curl_suffix_curl_off_t_test "unknown")
    if(suffix MATCHES "i64|i32|i16")
      set(curl_test_suffix_u "u${suffix}")
    endif(suffix MATCHES "i64|i32|i16")
    if(suffix MATCHES "LL|L")
      set(curl_test_suffix_u "U${suffix}")
    endif(suffix MATCHES "LL|L")
    message(STATUS "testing ${suffix} ${curl_test_suffix_u}") 
    set(EXTRA_DEFINES "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}")
    curl_check_c_source_compiles("
    typedef ${curl_typeof_curl_off_t} new_t;
    new_t s1;
    new_t s2;
    s1 = -10${suffix};
    s2 = 20${suffix};
    if(s1 > s2)
      return 1;
    " curl_suffix_curl_off_t_test)
    if(curl_suffix_curl_off_t_test)
      set(CURL_SUFFIX_CURL_OFF_T ${suffix} CACHE INTERNAL "signed suffix for off_t")
      set(CURL_SUFFIX_CURL_OFF_TU ${curl_test_suffix_u} CACHE INTERNAL "unsigned suffix for off_t")
    endif(curl_suffix_curl_off_t_test)
  endforeach(suffix)
  if(NOT DEFINED CURL_SUFFIX_CURL_OFF_T)
    set(CURL_SUFFIX_CURL_OFF_T "" CACHE INTERNAL "signed suffix for off_t")
    set(CURL_SUFFIX_CURL_OFF_TU "" CACHE INTERNAL "unsigned suffix for off_t")
  endif()
endfunction(check_curl_off_t_suffix)
# find the suffix to add to a literal number for the off_t type
check_curl_off_t_suffix()
message(STATUS "CURL_SUFFIX_CURL_OFF_T = ${CURL_SUFFIX_CURL_OFF_T} ")
message(STATUS "CURL_SUFFIX_CURL_OFF_TU = ${CURL_SUFFIX_CURL_OFF_TU} ")
message(STATUS "CURL_TYPEOF_CURL_OFF_T = ${CURL_TYPEOF_CURL_OFF_T} ")
message(STATUS "CURL_FORMAT_CURL_OFF_T = ${CURL_FORMAT_CURL_OFF_T} ")
message(STATUS "CURL_FORMAT_OFF_T = ${CURL_FORMAT_OFF_T}")
curl_check_c_source_compiles("recv(0, 0, 0, 0)" curl_cv_recv)
if(curl_cv_recv)
  #    AC_CACHE_CHECK([types of arguments and return type for recv],
  #[curl_cv_func_recv_args], [
  #SET(curl_cv_func_recv_args "unknown")
  #for recv_retv in 'int' 'ssize_t'; do
  if(NOT DEFINED curl_cv_func_recv_args OR "${curl_cv_func_recv_args}" STREQUAL "unknown")
    foreach(recv_retv "int" "ssize_t" )
      foreach(recv_arg1 "int" "ssize_t" "SOCKET")
        foreach(recv_arg2 "void *" "char *")
          foreach(recv_arg3 "size_t" "int" "socklen_t" "unsigned int")
            foreach(recv_arg4 "int" "unsigned int")
              if(NOT curl_cv_func_recv_done)
                set(curl_cv_func_recv_test "UNKNOWN")
                set(extern_line "extern ${recv_retv} ${signature_call_conv} recv(${recv_arg1}, ${recv_arg2}, ${recv_arg3}, ${recv_arg4})\;")
                set(EXTRA_DEFINES "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}\n#define __unused5")
                curl_check_c_source_compiles("
                    ${recv_arg1} s=0;
                    ${recv_arg2} buf=0;
                    ${recv_arg3} len=0;
                    ${recv_arg4} flags=0;
                    ${recv_retv} res = recv(s, buf, len, flags)"
                    curl_cv_func_recv_test
                    "${recv_retv} recv(${recv_arg1}, ${recv_arg2}, ${recv_arg3}, ${recv_arg4})")
                if(curl_cv_func_recv_test)
                  set(curl_cv_func_recv_args
                    "${recv_arg1},${recv_arg2},${recv_arg3},${recv_arg4},${recv_retv}")
                  set(RECV_TYPE_ARG1 "${recv_arg1}")
                  set(RECV_TYPE_ARG2 "${recv_arg2}")
                  set(RECV_TYPE_ARG3 "${recv_arg3}")
                  set(RECV_TYPE_ARG4 "${recv_arg4}")
                  set(RECV_TYPE_RETV "${recv_retv}")
                  set(HAVE_RECV 1)
                  set(curl_cv_func_recv_done 1)
                endif(curl_cv_func_recv_test)
              endif(NOT curl_cv_func_recv_done)
            endforeach(recv_arg4)
          endforeach(recv_arg3)
        endforeach(recv_arg2)
      endforeach(recv_arg1)
    endforeach(recv_retv) 
  else(NOT DEFINED curl_cv_func_recv_args OR "${curl_cv_func_recv_args}" STREQUAL "unknown")
    string(REGEX REPLACE "^([^,]*),[^,]*,[^,]*,[^,]*,[^,]*$" "\\1" RECV_TYPE_ARG1 "${curl_cv_func_recv_args}")
    string(REGEX REPLACE "^[^,]*,([^,]*),[^,]*,[^,]*,[^,]*$" "\\1" RECV_TYPE_ARG2 "${curl_cv_func_recv_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,([^,]*),[^,]*,[^,]*$" "\\1" RECV_TYPE_ARG3 "${curl_cv_func_recv_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,[^,]*,([^,]*),[^,]*$" "\\1" RECV_TYPE_ARG4 "${curl_cv_func_recv_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,[^,]*,[^,]*,([^,]*)$" "\\1" RECV_TYPE_RETV "${curl_cv_func_recv_args}")
    #MESSAGE("RECV_TYPE_ARG1 ${RECV_TYPE_ARG1}")
    #MESSAGE("RECV_TYPE_ARG2 ${RECV_TYPE_ARG2}")
    #MESSAGE("RECV_TYPE_ARG3 ${RECV_TYPE_ARG3}")
    #MESSAGE("RECV_TYPE_ARG4 ${RECV_TYPE_ARG4}")
    #MESSAGE("RECV_TYPE_RETV ${RECV_TYPE_RETV}")
  endif(NOT DEFINED curl_cv_func_recv_args OR "${curl_cv_func_recv_args}" STREQUAL "unknown")
  
  if("${curl_cv_func_recv_args}" STREQUAL "unknown")
    message(FATAL_ERROR "Cannot find proper types to use for recv args")
  endif("${curl_cv_func_recv_args}" STREQUAL "unknown")
else(curl_cv_recv)
  message(FATAL_ERROR "Unable to link function recv")
endif(curl_cv_recv)
set(curl_cv_func_recv_args "${curl_cv_func_recv_args}" CACHE INTERNAL "Arguments for recv")
set(HAVE_RECV 1)

curl_check_c_source_compiles("send(0, 0, 0, 0)" curl_cv_send)
if(curl_cv_send)
  #    AC_CACHE_CHECK([types of arguments and return type for send],
  #[curl_cv_func_send_args], [
  #SET(curl_cv_func_send_args "unknown")
  #for send_retv in 'int' 'ssize_t'; do
  if(NOT DEFINED curl_cv_func_send_args OR "${curl_cv_func_send_args}" STREQUAL "unknown")
    foreach(send_retv "int" "ssize_t" )
      foreach(send_arg1 "int" "ssize_t" "SOCKET")
        foreach(send_arg2 "const void *" "void *" "char *" "const char *")
          foreach(send_arg3 "size_t" "int" "socklen_t" "unsigned int")
            foreach(send_arg4 "int" "unsigned int")
              if(NOT curl_cv_func_send_done)
                set(curl_cv_func_send_test "UNKNOWN")
                set(extern_line "extern ${send_retv} ${signature_call_conv} send(${send_arg1}, ${send_arg2}, ${send_arg3}, ${send_arg4})\;")
                set(EXTRA_DEFINES "${EXTRA_DEFINES_BACKUP}\n${headers_hack}\n${extern_line}\n#define __unused5")
                curl_check_c_source_compiles("
                    ${send_arg1} s=0;
                    ${send_arg2} buf=0;
                    ${send_arg3} len=0;
                    ${send_arg4} flags=0;
                    ${send_retv} res = send(s, buf, len, flags)"
                    curl_cv_func_send_test
                    "${send_retv} send(${send_arg1}, ${send_arg2}, ${send_arg3}, ${send_arg4})")
                if(curl_cv_func_send_test)
                  #MESSAGE("Found arguments: ${curl_cv_func_send_test}")
                  string(REGEX REPLACE "(const) .*" "\\1" send_qual_arg2 "${send_arg2}")
                  string(REGEX REPLACE "const (.*)" "\\1" send_arg2 "${send_arg2}")
                  set(curl_cv_func_send_args
                    "${send_arg1},${send_arg2},${send_arg3},${send_arg4},${send_retv},${send_qual_arg2}")
                  set(SEND_TYPE_ARG1 "${send_arg1}")
                  set(SEND_TYPE_ARG2 "${send_arg2}")
                  set(SEND_TYPE_ARG3 "${send_arg3}")
                  set(SEND_TYPE_ARG4 "${send_arg4}")
                  set(SEND_TYPE_RETV "${send_retv}")
                  set(HAVE_SEND 1)
                  set(curl_cv_func_send_done 1)
                endif(curl_cv_func_send_test)
              endif(NOT curl_cv_func_send_done)
            endforeach(send_arg4)
          endforeach(send_arg3)
        endforeach(send_arg2)
      endforeach(send_arg1)
    endforeach(send_retv) 
  else(NOT DEFINED curl_cv_func_send_args OR "${curl_cv_func_send_args}" STREQUAL "unknown")
    string(REGEX REPLACE "^([^,]*),[^,]*,[^,]*,[^,]*,[^,]*,[^,]*$" "\\1" SEND_TYPE_ARG1 "${curl_cv_func_send_args}")
    string(REGEX REPLACE "^[^,]*,([^,]*),[^,]*,[^,]*,[^,]*,[^,]*$" "\\1" SEND_TYPE_ARG2 "${curl_cv_func_send_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,([^,]*),[^,]*,[^,]*,[^,]*$" "\\1" SEND_TYPE_ARG3 "${curl_cv_func_send_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,[^,]*,([^,]*),[^,]*,[^,]*$" "\\1" SEND_TYPE_ARG4 "${curl_cv_func_send_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,[^,]*,[^,]*,([^,]*),[^,]*$" "\\1" SEND_TYPE_RETV "${curl_cv_func_send_args}")
    string(REGEX REPLACE "^[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,([^,]*)$" "\\1" SEND_QUAL_ARG2 "${curl_cv_func_send_args}")
    #MESSAGE("SEND_TYPE_ARG1 ${SEND_TYPE_ARG1}")
    #MESSAGE("SEND_TYPE_ARG2 ${SEND_TYPE_ARG2}")
    #MESSAGE("SEND_TYPE_ARG3 ${SEND_TYPE_ARG3}")
    #MESSAGE("SEND_TYPE_ARG4 ${SEND_TYPE_ARG4}")
    #MESSAGE("SEND_TYPE_RETV ${SEND_TYPE_RETV}")
    #MESSAGE("SEND_QUAL_ARG2 ${SEND_QUAL_ARG2}")
  endif(NOT DEFINED curl_cv_func_send_args OR "${curl_cv_func_send_args}" STREQUAL "unknown")
  
  if("${curl_cv_func_send_args}" STREQUAL "unknown")
    message(FATAL_ERROR "Cannot find proper types to use for send args")
  endif("${curl_cv_func_send_args}" STREQUAL "unknown")
  set(SEND_QUAL_ARG2 "const")
else(curl_cv_send)
  message(FATAL_ERROR "Unable to link function send")
endif(curl_cv_send)
set(curl_cv_func_send_args "${curl_cv_func_send_args}" CACHE INTERNAL "Arguments for send")
set(HAVE_SEND 1)

set(EXTRA_DEFINES "${EXTRA_DEFINES}\n${headers_hack}\n#define __unused5")
curl_check_c_source_compiles("int flag = MSG_NOSIGNAL" HAVE_MSG_NOSIGNAL)

set(EXTRA_DEFINES "__unused1\n#undef inline\n#define __unused2")
set(HEADER_INCLUDES)
set(headers_hack)

macro(add_header_include check header)
  if(${check})
    set(headers_hack
      "${headers_hack}\n#include <${header}>")
    #SET(HEADER_INCLUDES
    #  ${HEADER_INCLUDES}
    #  "${header}")
  endif(${check})
endmacro(add_header_include header)

if(HAVE_WINDOWS_H)
  set(EXTRA_DEFINES ${EXTRA_DEFINES}
    "__unused7\n#ifndef WIN32_LEAN_AND_MEAN\n#define WIN32_LEAN_AND_MEAN\n#endif\n#define __unused3")
  add_header_include(HAVE_WINDOWS_H "windows.h")
  add_header_include(HAVE_WINSOCK2_H "winsock2.h")
  add_header_include(HAVE_WINSOCK_H "winsock.h")
else(HAVE_WINDOWS_H)
  add_header_include(HAVE_SYS_TYPES_H "sys/types.h")
  add_header_include(HAVE_SYS_TIME_H "sys/time.h")
  add_header_include(TIME_WITH_SYS_TIME "time.h")
  add_header_include(HAVE_TIME_H "time.h")
endif(HAVE_WINDOWS_H)
set(EXTRA_DEFINES "${EXTRA_DEFINES}\n${headers_hack}\n#define __unused5")
curl_check_c_source_compiles("struct timeval ts;\nts.tv_sec  = 0;\nts.tv_usec = 0" HAVE_STRUCT_TIMEVAL)


include(CurlCheckCSourceRuns)
set(EXTRA_DEFINES)
set(HEADER_INCLUDES)
if(HAVE_SYS_POLL_H)
  set(HEADER_INCLUDES "sys/poll.h")
endif(HAVE_SYS_POLL_H)
curl_check_c_source_runs("return poll((void *)0, 0, 10 /*ms*/)" HAVE_POLL_FINE)

set(HAVE_SIG_ATOMIC_T 1)
set(EXTRA_DEFINES)
set(HEADER_INCLUDES)
if(HAVE_SIGNAL_H)
  set(HEADER_INCLUDES "signal.h")
  set(CMAKE_EXTRA_INCLUDE_FILES "signal.h")
endif(HAVE_SIGNAL_H)
check_type_size("sig_atomic_t" SIZEOF_SIG_ATOMIC_T)
if(HAVE_SIZEOF_SIG_ATOMIC_T)
  curl_check_c_source_compiles("static volatile sig_atomic_t dummy = 0" HAVE_SIG_ATOMIC_T_NOT_VOLATILE)
  if(NOT HAVE_SIG_ATOMIC_T_NOT_VOLATILE)
    set(HAVE_SIG_ATOMIC_T_VOLATILE 1)
  endif(NOT HAVE_SIG_ATOMIC_T_NOT_VOLATILE)
endif(HAVE_SIZEOF_SIG_ATOMIC_T)

set(CHECK_TYPE_SIZE_PREINCLUDE
  "#undef inline")

if(HAVE_WINDOWS_H)
  set(CHECK_TYPE_SIZE_PREINCLUDE "${CHECK_TYPE_SIZE_PREINCLUDE}
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>")
  if(HAVE_WINSOCK2_H)
    set(CHECK_TYPE_SIZE_PREINCLUDE "${CHECK_TYPE_SIZE_PREINCLUDE}\n#include <winsock2.h>")
  endif(HAVE_WINSOCK2_H)
else(HAVE_WINDOWS_H)
  if(HAVE_SYS_SOCKET_H)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES}
      "sys/socket.h")
  endif(HAVE_SYS_SOCKET_H)
  if(HAVE_NETINET_IN_H)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES}
      "netinet/in.h")
  endif(HAVE_NETINET_IN_H)
  if(HAVE_ARPA_INET_H)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES}
      "arpa/inet.h")
  endif(HAVE_ARPA_INET_H)
endif(HAVE_WINDOWS_H)

check_type_size("struct sockaddr_storage" SIZEOF_STRUCT_SOCKADDR_STORAGE)
if(HAVE_SIZEOF_STRUCT_SOCKADDR_STORAGE)
  set(HAVE_STRUCT_SOCKADDR_STORAGE 1)
endif(HAVE_SIZEOF_STRUCT_SOCKADDR_STORAGE)

