
# helper macro for test clean-up
macro(CLEAN)
  unset(RegistryView_DIR CACHE)
  unset(RegistryView_FOUND)
  unset(RegistryView64_DIR CACHE)
  unset(RegistryView64_FOUND)
  unset(RegistryView32_DIR CACHE)
  unset(RegistryView32_FOUND)
endmacro()


cmake_policy(SET CMP0134 NEW)

# HKCU/Software/Classes/CLSID/CMake-Tests/find_package: Query default value
set(FILE_DIR "[HKCU/Software/Classes/CLSID/CMake-Tests/find_package]")
set(FILE_DIR2 "[HKCU/Software/Classes/CLSID/CMake-Tests/find_package;(default)]")

set(EXPECTED_LOCATION "default.${ARCH}")

find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_DEFAULT_PATH)
clean()

# query value using special name should be identical to default value
find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR2}" REQUIRED NO_DEFAULT_PATH)
clean()

find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_DEFAULT_PATH)
clean()

# VIEW TARGET should have same value as VIEW HOST
find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_DEFAULT_PATH)
clean()

if (ARCH STREQUAL "64bit")

  set(EXPECTED_LOCATION "default.64bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "default.32bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "default.64bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "default.32bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the second view is taken into account
  set(EXPECTED_LOCATION "default.32bit")
  find_package(RegistryView32 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "default.64bit")
  find_package(RegistryView64 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the both views are taken into account
  set(EXPECTED_LOCATION "default.32bit")
  find_package(RegistryView32 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "default.64bit")
  find_package(RegistryView64 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

else() # 32bit

  # no 64bit registry: file not found
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 NO_DEFAULT_PATH)
  if (RegistryView_FOUND)
    message (SEND_ERROR "Unexpectedly found file '${RegistryView_DIR}/RegistryViewConfog.cmake'")
  endif()
  clean()

  set(EXPECTED_LOCATION "default.32bit")

  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_DEFAULT_PATH)
  clean()

  # views 64_32 and 32_64 give same result
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  find_package(RegistryView PATHS "${CMAKE_ CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the both views are usable on 32bit platforms
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

endif()


# HKCU/Software/Classes/CLSID/CMake-Tests/find_package: Query specific value
set(FILE_DIR "[{|}HKCU/Software/Classes/CLSID/CMake-Tests/find_package|FILE_DIR]")
set(FILE_DIR2 "[HKCU\\Software\\Classes\\CLSID\\CMake-Tests\\find_package;FILE_DIR]")

set(EXPECTED_LOCATION "${ARCH}")

find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REQUIRED NO_DEFAULT_PATH)
clean()

# query value using special name should be identical to default value
find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR2}" REQUIRED NO_DEFAULT_PATH)
clean()

find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST REQUIRED NO_DEFAULT_PATH)
clean()
# VIEW TARGET should have same value as VIEW HOST
find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET REQUIRED NO_DEFAULT_PATH)
clean()

if (ARCH STREQUAL "64bit")

  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "32bit")
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "32bit")
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the second view is taken into account
  find_package(RegistryView32 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView64 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the both views are taken into account
  set(EXPECTED_LOCATION "32bit")
  find_package(RegistryView32 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView64 NAMES HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

else() # 32bit

  # no 64bit registry: file not found
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64 NO_DEFAULT_PATH)
  if (RegistryView_FOUND)
    message (SEND_ERROR "Unexpectedly found file '${RegistryView_DIR}/RegistryViewConfog.cmake'")
  endif()
  clean()

  set(EXPECTED_LOCATION "32bit")

  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32 REQUIRED NO_DEFAULT_PATH)
  clean()

  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 64_32 REQUIRED NO_DEFAULT_PATH)
  clean()

  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW 32_64 REQUIRED NO_DEFAULT_PATH)
  clean()

  # check the both views are taken into account
  find_package(RegistryView HINTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW BOTH REQUIRED NO_DEFAULT_PATH)
  clean()

endif()

if (ARCH STREQUAL "64bit")

  # Check influence of variable CMAKE_SIZEOF_VOID_P
  set(CMAKE_SIZEOF_VOID_P 8)
  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET)
  clean()

  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST)
  clean()


  set(CMAKE_SIZEOF_VOID_P 4)
  set(EXPECTED_LOCATION "32bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW TARGET)
  clean()

  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}" REGISTRY_VIEW HOST)
  clean()

  unset(CMAKE_SIZEOF_VOID_P)


  # Check influence of CMP0134 policy with OLD value
  cmake_policy(SET CMP0134 OLD)
  # CMAKE_SIZEOF_VOID_P is not set, so search first 32bit registry
  set(EXPECTED_LOCATION "32bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}")
  clean()

  cmake_policy(SET CMP0134 NEW)
  # CMAKE_SIZEOF_VOID_P is not set, so search first the HOST architecture registry
  set(EXPECTED_LOCATION "64bit")
  find_package(RegistryView PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_DIR}")
  clean()

endif()
