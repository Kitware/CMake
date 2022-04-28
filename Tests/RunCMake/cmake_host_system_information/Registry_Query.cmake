
# helper function for test validation
function(CHECK key result status expression)
  if(status STREQUAL "")
  cmake_language(EVAL CODE
    "if (NOT (${expression}))
       message(SEND_ERROR \"wrong value for key '${key}': '${result}'\")
     endif()")
  else()
    message(SEND_ERROR "query failed for key '${key}': '${status}'")
  endif()
endfunction()


# HKCU/Software/Classes/CLSID/CMake-Tests/chsi-registry: Query default value
set(KEY "HKCU/Software/Classes/CLSID/CMake-Tests/chsi-registry")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" ERROR_VARIABLE status)
check("${KEY}" "${result}" "${status}"
      "result STREQUAL \"default ${ARCH}\"")
# query value using special name should be identical to default value
cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE "(default)" ERROR_VARIABLE status)
check("${KEY}{(default)}" "${result2}" "${status}" "result2 STREQUAL result")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW HOST ERROR_VARIABLE status)
check("${KEY}" "${result}" "${status}"
      "result STREQUAL \"default ${ARCH}\"")
# VIEW TARGET should have same value as VIEW HOST
cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VIEW TARGET ERROR_VARIABLE status)
check("${KEY}" "${result2}" "${status}" "result2 STREQUAL result")

if (ARCH STREQUAL "64bit")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 64 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 64bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 32 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 64bit\"")

  # reg 32bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

else() #32bit

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 64 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 32 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

  # reg 32bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

endif()


# HKCU/Software/CMake-Tests/chsi-registry: Query named value
set(KEY "HKCU/Software/Classes/CLSID/CMake-Tests/chsi-registry")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
                              ERROR_VARIABLE status)
check("${KEY}{BYTE_SIZE}" "${result}" "${status}"
      "result STREQUAL \"${ARCH}\"")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
                              VIEW HOST ERROR_VARIABLE status)
check("${KEY}{BYTE_SIZE}" "${result}" "${status}"
      "result STREQUAL \"${ARCH}\"")
# VIEW TARGET should have same value as VIEW HOST
cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
                              VIEW TARGET ERROR_VARIABLE status)
check("${KEY}{BYTE_SIZE}" "${result2}" "${status}" "result2 STREQUAL result")

if (ARCH STREQUAL "64bit")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"64bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"32bit\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"64bit\"")

  # reg 32bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"32bit\"")

else() # 32bit

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"32bit\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"32bit\"")

  # reg 32bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}{BYTE_SIZE}" "${result}" "${status}" "result STREQUAL \"32bit\"")

endif()

# HKCU/Software/CMake-Tests/chsi-registry: check retrieval of various types
cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE_SZ ERROR_VARIABLE status)
check("${KEY}{VALUE_SZ}" "${result}" "${status}" "result STREQUAL \"data with space\"")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE_EXPAND_SZ ERROR_VARIABLE status)
check("${KEY}{VALUE_EXPAND_SZ}" "${result}" "${status}"
      "(NOT result STREQUAL \"PATH=%PATH%\") AND (result MATCHES \"^PATH=\")")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE_MULTI_SZ ERROR_VARIABLE status)
check("${KEY}{VALUE_MULTI_SZ}" "${result}" "${status}" "result STREQUAL \"data1;data2\"")
cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE2_MULTI_SZ
                              SEPARATOR "|" ERROR_VARIABLE status)
check("${KEY}{VALUE2_MULTI_SZ}" "${result}" "${status}" "result STREQUAL \"data1;data2\"")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE_DWORD ERROR_VARIABLE status)
check("${KEY}{VALUE_DWORD}" "${result}" "${status}" "result EQUAL \"129\"")

cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE VALUE_QWORD ERROR_VARIABLE status)
check("${KEY}{VALUE_QWORD}" "${result}" "${status}" "result EQUAL \"513\"")


# HKCU/Software/CMake-Tests/chsi-registry: check retrieval of value names
if (ARCH STREQUAL "64bit")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE2_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")
  # VIEW BOTH should have same result as default view
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW BOTH ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result2 STREQUAL result")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW HOST ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}"
    "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")
  # VIEW TARGET should have same result as VIEW HOST
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result2 STREQUAL result")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}"
    "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_SZ\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}"
    "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE2_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")

  # reg 32bit is read first. Result is the same as with view 64_32
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result2 STREQUAL result")

else()

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")
  # VIEW BOTH should have same result as default view
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW BOTH ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result STREQUAL result2")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW HOST ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}"
    "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ")
  # VIEW TARGET should have same result as VIEW HOST
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result2 STREQUAL result")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}" "result STREQUAL \"\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result}" "${status}"
    "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")

  # reg 32bit is read first. Result is the same as with view 64_32
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}[VALUE_NAMES]" "${result2}" "${status}" "result2 STREQUAL result")

endif()


# HKCU/Software/CMake-Tests/chsi-registry: check retrieval of sub keys
if (ARCH STREQUAL "64bit")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2;subkey3\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW HOST ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")
  # VIEW TARGET should have same result as VIEW HOST
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result2 STREQUAL result")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey3\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}[SUBLEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2;subkey3\"")

  # reg 32bit is read first. Result is the same as with view 64_32
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result2}" "${status}" "result2 STREQUAL result")

else()

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW HOST ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")
  # VIEW TARGET should have same result as VIEW HOST
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result2 STREQUAL result")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 64 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 32 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")

  # reg 64bit is read first
  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 64_32 ERROR_VARIABLE status)
  check("${KEY}[SUBLEYS]" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")

  # reg 32bit is read first. Result is the same as with view 64_32
  cmake_host_system_information(RESULT result2 QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW 32_64 ERROR_VARIABLE status)
  check("${KEY}[SUBKEYS]" "${result2}" "${status}" "result2 STREQUAL result")

endif()


if (ARCH STREQUAL "64bit")

  # Check influence of variable CMAKE_SIZEOF_VOID_P
  set(CMAKE_SIZEOF_VOID_P 8)

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}"
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 64bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"64bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_MULTI_SZ;VALUE_DWORD;VALUE_EXPAND_SZ;VALUE_MULTI_SZ;VALUE_QWORD;VALUE_SZ\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"subkey1;subkey2\"")


  set(CMAKE_SIZEOF_VOID_P 4)

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}"
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"default 32bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE BYTE_SIZE
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"32bit\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" VALUE_NAMES
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"(default);BYTE_SIZE;VALUE2_SZ\"")

  cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY "${KEY}" SUBKEYS
    VIEW TARGET ERROR_VARIABLE status)
  check("${KEY}" "${result}" "${status}" "result STREQUAL \"subkey1;subkey3\"")

endif()
