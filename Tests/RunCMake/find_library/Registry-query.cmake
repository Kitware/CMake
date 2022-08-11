
# helper function for test validation
function(CHECK query result expression)
  cmake_language(EVAL CODE
    "if (NOT (${expression}))
       message(SEND_ERROR \"wrong value for query '${query}': '${result}'\")
     endif()")
endfunction()

cmake_policy(SET CMP0134 NEW)

# HKCU/Software/Classes/CLSID/CMake-Tests/find_library: Query default value
set(FILE_DIR "[HKCU/Software/Classes/CLSID/CMake-Tests/find_library]")
set(FILE_DIR2 "[HKCU/Software/Classes/CLSID/CMake-Tests/find_library;(default)]")

unset(result)
find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result}" "result MATCHES \"default.${ARCH}/file.lib$\"")

# query value using special name should be identical to default value
unset(result)
find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR2}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR2}" "${result}" "result MATCHES \"default.${ARCH}/file.lib$\"")

unset(result)
find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result}" "result MATCHES \"default.${ARCH}/file.lib$\"")
# VIEW TARGET should have same value as VIEW HOST
unset(result2)
find_library(result2 NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result2}" "result2 STREQUAL result")

if (ARCH STREQUAL "64bit")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.64bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.64bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

  # check the second view is taken into account
  unset(result)
  find_library(result NAMES file32bit.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file32bit.lib$\"")

  unset(result)
  find_library(result NAMES file64bit.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.64bit/file64bit.lib$\"")
  unset(result)

  # check the both views are taken into account
  unset(result)
  find_library(result NAMES file32bit.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file32bit.lib$\"")

  unset(result)
  find_library(result NAMES file64bit.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.64bit/file64bit.lib$\"")
  unset(result)

else() # 32bit

  # no 64bit registry: file not found
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"result-NOTFOUND$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

  # views 64_32 and 32_64 give same result
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

  # check the both views are usable on 32bit platforms
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"default.32bit/file.lib$\"")

endif()

# HKCU/Software/Classes/CLSID/CMake-Tests/find_library: Query specific value
set(FILE_DIR "[{|}HKCU/Software/Classes/CLSID/CMake-Tests/find_library|FILE_DIR]")
set(FILE_DIR2 "[HKCU\\Software\\Classes\\CLSID\\CMake-Tests\\find_library;FILE_DIR]")

unset(result)
find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")

# query value using special name should be identical to default value
unset(result)
find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR2}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR2}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")

unset(result)
find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")
# VIEW TARGET should have same value as VIEW HOST
unset(result2)
find_library(result2 NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_CACHE NO_DEFAULT_PATH)
check("${FILE_DIR}" "${result2}" "result2 STREQUAL result")

if (ARCH STREQUAL "64bit")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/64bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/64bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  # check the second view is taken into account
  unset(result)
  find_library(result NAMES file32bit.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file32bit.lib$\"")

  unset(result)
  find_library(result NAMES file64bit.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/64bit/file64bit.lib$\"")

  # check the both views are taken into account
  unset(result)
  find_library(result NAMES file32bit.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file32bit.lib$\"")

  unset(result)
  find_library(result NAMES file64bit.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/64bit/file64bit.lib$\"")

else() # 32bit

  # no 64bit registry: file not found
  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"result-NOTFOUND$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

endif()

if (ARCH STREQUAL "64bit")

  # Check influence of variable CMAKE_SIZEOF_VOID_P
  set(CMAKE_SIZEOF_VOID_P 8)
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/64bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")


  set(CMAKE_SIZEOF_VOID_P 4)
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")

  unset(CMAKE_SIZEOF_VOID_P)


  # Check influence of CMP0134 policy with OLD value
  cmake_policy(SET CMP0134 OLD)
  # CMAKE_SIZEOF_VOID_P is not set, so search first 32bit registry
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/32bit/file.lib$\"")

  cmake_policy(SET CMP0134 NEW)
  # CMAKE_SIZEOF_VOID_P is not set, so search first the HOST architecture registry
  unset(result)
  find_library(result NAMES file.lib PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_CACHE NO_DEFAULT_PATH)
  check("${FILE_DIR}" "${result}" "result MATCHES \"/${ARCH}/file.lib$\"")

endif()
