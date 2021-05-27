# Hard-code architecture for test without a real compiler.
set(CMAKE_SIZEOF_VOID_P 4)

include(WriteBasicConfigVersionFile)

set(COMPATIBILITIES AnyNewerVersion
                    SameMajorVersion
                    SameMinorVersion
                    ExactVersion
                    )

function(TEST_WRITE_BASIC_CONFIG_VERSION_FILE_PREPARE _version_installed)
  set(_same_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
  set(_no_CMAKE_SIZEOF_VOID_P "")
  math(EXPR _diff_CMAKE_SIZEOF_VOID_P "${CMAKE_SIZEOF_VOID_P} + 1")
  foreach(_compat ${COMPATIBILITIES})
    set(_pkg ${_compat}${_version_installed})
    string(REPLACE "." "" _pkg ${_pkg})
    set(_filename "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}ConfigVersion.cmake")
    set(_filename_novoid "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}NoVoidConfigVersion.cmake")
    set(_filename_diffvoid "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}DiffVoidConfigVersion.cmake")

    set(CMAKE_SIZEOF_VOID_P ${_same_CMAKE_SIZEOF_VOID_P})
    write_basic_config_version_file("${_filename}"
                                    VERSION ${_version_installed}
                                    COMPATIBILITY ${_compat})

    # Test that an empty CMAKE_SIZEOF_VOID_P is accepted:
    set(CMAKE_SIZEOF_VOID_P ${_no_CMAKE_SIZEOF_VOID_P})
    write_basic_config_version_file("${_filename_novoid}"
                                    VERSION ${_version_installed}
                                    COMPATIBILITY ${_compat})

    # Test that a different CMAKE_SIZEOF_VOID_P results in
    # PACKAGE_VERSION_UNSUITABLE
    set(CMAKE_SIZEOF_VOID_P ${_diff_CMAKE_SIZEOF_VOID_P})
    write_basic_config_version_file("${_filename_diffvoid}"
                                    VERSION ${_version_installed}
                                    COMPATIBILITY ${_compat})
  endforeach()
endfunction()

macro(TEST_WRITE_BASIC_CONFIG_VERSION_FILE_CHECK _filename)
  include("${_filename}")

  message(STATUS "_expected_compatible: ${_expected_compatible}")
  message(STATUS "_expected_unsuitable: ${_expected_unsuitable}")
  if(_expected_compatible AND NOT PACKAGE_VERSION_COMPATIBLE)
    message(SEND_ERROR "Did not find package with version ${_version_installed} (${_version_requested} was requested)!")
  elseif(NOT _expected_compatible AND PACKAGE_VERSION_COMPATIBLE)
    message(SEND_ERROR "Found package with version ${_version_installed}, but ${_version_requested} was requested!")
  endif()

  if(${_expected_unsuitable} AND NOT PACKAGE_VERSION_UNSUITABLE)
    message(SEND_ERROR "PACKAGE_VERSION_UNSUITABLE set, although it should not be!")
  elseif(NOT ${_expected_unsuitable} AND PACKAGE_VERSION_UNSUITABLE)
    message(SEND_ERROR "PACKAGE_VERSION_UNSUITABLE not set, although it should be!")
  endif()

  unset(PACKAGE_VERSION_COMPATIBLE)
  unset(PACKAGE_VERSION_EXACT)
  unset(PACKAGE_VERSION_UNSUITABLE)
endmacro()

function(TEST_WRITE_BASIC_CONFIG_VERSION_FILE _version_installed
                                              _version_requested
                                              _expected_compatible_AnyNewerVersion
                                              _expected_compatible_SameMajorVersion
                                              _expected_compatible_SameMinorVersion
                                              _expected_compatible_ExactVersion)
  if("${_version_requested}" MATCHES [[^([0-9]+(\.[0-9]+)*)\.\.\.(<)?([0-9]+(\.[0-9]+)*)$]])
    set (_compatibilities ${COMPATIBILITIES})
    # ExactVersion must not be tested
    list(POP_BACK _compatibilities)
    set(PACKAGE_FIND_VERSION_RANGE TRUE)
    set(PACKAGE_FIND_VERSION_RANGE_MIN INCLUDE)
    if ("${CMAKE_MATCH_3}" STREQUAL "<")
      set(PACKAGE_FIND_VERSION_RANGE_MAX EXCLUDE)
    else()
      set(PACKAGE_FIND_VERSION_RANGE_MAX INCLUDE)
    endif()
    set(PACKAGE_FIND_VERSION_MIN "${CMAKE_MATCH_1}")
    set(PACKAGE_FIND_VERSION_MAX "${CMAKE_MATCH_4}")
    if("${PACKAGE_FIND_VERSION_MIN}" MATCHES [[(^([0-9]+)(\.([0-9]+)(\.([0-9]+)(\.([0-9]+))?)?)?)?$]])
      set(PACKAGE_FIND_VERSION_MIN_MAJOR "${CMAKE_MATCH_2}")
      set(PACKAGE_FIND_VERSION_MIN_MINOR "${CMAKE_MATCH_4}")
      set(PACKAGE_FIND_VERSION_MIN_PATCH "${CMAKE_MATCH_6}")
      set(PACKAGE_FIND_VERSION_MIN_TWEAK "${CMAKE_MATCH_8}")
    else()
      message(FATAL_ERROR "_version_requested (${_version_requested}) should be a version range")
    endif()
    set(PACKAGE_FIND_VERSION "${PACKAGE_FIND_VERSION_MIN}")
    set(PACKAGE_FIND_VERSION_MAJOR "${PACKAGE_FIND_VERSION_MIN_MAJOR}")
    set(PACKAGE_FIND_VERSION_MINOR "${PACKAGE_FIND_VERSION_MIN_MINOR}")
    set(PACKAGE_FIND_VERSION_PATCH "${PACKAGE_FIND_VERSION_MIN_PATCH}")
    set(PACKAGE_FIND_VERSION_TWEAK "${PACKAGE_FIND_VERSION_MIN_TWEAK}")
    if("${PACKAGE_FIND_VERSION_MAX}" MATCHES [[(^([0-9]+)(\.([0-9]+)(\.([0-9]+)(\.([0-9]+))?)?)?)?$]])
      set(PACKAGE_FIND_VERSION_MAX_MAJOR "${CMAKE_MATCH_2}")
      set(PACKAGE_FIND_VERSION_MAX_MINOR "${CMAKE_MATCH_4}")
      set(PACKAGE_FIND_VERSION_MAX_PATCH "${CMAKE_MATCH_6}")
      set(PACKAGE_FIND_VERSION_MAX_TWEAK "${CMAKE_MATCH_8}")
    else()
      message(FATAL_ERROR "_version_requested (${_version_requested}) should be a version range")
    endif()
  else()
    set (_compatibilities ${COMPATIBILITIES})
    set(PACKAGE_FIND_VERSION ${_version_requested})
    if("${PACKAGE_FIND_VERSION}" MATCHES [[(^([0-9]+)(\.([0-9]+)(\.([0-9]+)(\.([0-9]+))?)?)?)?$]])
      set(PACKAGE_FIND_VERSION_MAJOR "${CMAKE_MATCH_2}")
      set(PACKAGE_FIND_VERSION_MINOR "${CMAKE_MATCH_4}")
      set(PACKAGE_FIND_VERSION_PATCH "${CMAKE_MATCH_6}")
      set(PACKAGE_FIND_VERSION_TWEAK "${CMAKE_MATCH_8}")
    else()
      message(FATAL_ERROR "_version_requested (${_version_requested}) should be a version number")
    endif()

  endif()

  unset(PACKAGE_VERSION_COMPATIBLE)
  unset(PACKAGE_VERSION_EXACT)
  unset(PACKAGE_VERSION_UNSUITABLE)

  foreach(_compat ${_compatibilities})
    set(_pkg ${_compat}${_version_installed})
    string(REPLACE "." "" _pkg ${_pkg})
    set(_filename "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}ConfigVersion.cmake")
    set(_filename_novoid "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}NoVoidConfigVersion.cmake")
    set(_filename_diffvoid "${CMAKE_CURRENT_BINARY_DIR}/${_pkg}DiffVoidConfigVersion.cmake")

    set(_expected_compatible ${_expected_compatible_${_compat}})

    # Test "normal" version
    set(_expected_unsuitable 0)
    message(STATUS "TEST write_basic_config_version_file(VERSION ${_version_installed} \
COMPATIBILITY ${_compat}) vs. ${_version_requested}                                 \
(expected compatible = ${_expected_compatible}, unsuitable = ${_expected_unsuitable})")
    test_write_basic_config_version_file_check("${_filename}")

    # test empty CMAKE_SIZEOF_VOID_P version:
    set(_expected_unsuitable 0)
    message(STATUS "TEST write_basic_config_version_file(VERSION ${_version_installed} \
COMPATIBILITY ${_compat}) vs. ${_version_requested} (no CMAKE_SIZEOF_VOID_P)        \
(expected compatible = ${_expected_compatible}, unsuitable = ${_expected_unsuitable})")
    test_write_basic_config_version_file_check("${_filename_novoid}")

    # test different CMAKE_SIZEOF_VOID_P version:
    set(_expected_unsuitable 1)
    message(STATUS "TEST write_basic_config_version_file(VERSION ${_version_installed} \
COMPATIBILITY ${_compat}) vs. ${_version_requested} (different CMAKE_SIZEOF_VOID_P) \
(expected compatible = ${_expected_compatible}, unsuitable = ${_expected_unsuitable})")
    test_write_basic_config_version_file_check("${_filename_diffvoid}")

  endforeach()
endfunction()


test_write_basic_config_version_file_prepare(4)
test_write_basic_config_version_file_prepare(4.05)
test_write_basic_config_version_file_prepare(4.5.06)
test_write_basic_config_version_file_prepare(4.05.06.007)

#                                                      AnyNewerVersion
#                                                      |  SameMajorVersion
#                                                      |  |  SameMinorVersion
#                                                      |  |  |  ExactVersion
#                                                      |  |  |  |
test_write_basic_config_version_file(4.05     0        1  0  0  0) # Request 0
test_write_basic_config_version_file(4.05     2        1  0  0  0) # Request [older major]
test_write_basic_config_version_file(4.05     4        1  1  0  0) # Request [same major]
test_write_basic_config_version_file(4.05     9        0  0  0  0) # Request [newer major]

test_write_basic_config_version_file(4.05     0.0      1  0  0  0) # Request 0.0
test_write_basic_config_version_file(4.05     0.2      1  0  0  0) # Request 0.[older minor]
test_write_basic_config_version_file(4.05     0.5      1  0  0  0) # Request 0.[same minor]
test_write_basic_config_version_file(4.05     0.9      1  0  0  0) # Request 0.[newer minor]
test_write_basic_config_version_file(4.05     2.0      1  0  0  0) # Request [older major].0
test_write_basic_config_version_file(4.05     2.2      1  0  0  0) # Request [older major].[older minor]
test_write_basic_config_version_file(4.05     2.5      1  0  0  0) # Request [older major].[same minor]
test_write_basic_config_version_file(4.05     2.9      1  0  0  0) # Request [older major].[newer minor]
test_write_basic_config_version_file(4.05     4.0      1  1  0  0) # Request [same major].0
test_write_basic_config_version_file(4.05     4.2      1  1  0  0) # Request [same major].[older minor]
test_write_basic_config_version_file(4.05     4.05      1  1  1  1) # Request [same major].[same minor]
test_write_basic_config_version_file(4.05     4.9      0  0  0  0) # Request [same major].[newer minor]
test_write_basic_config_version_file(4.05     9.0      0  0  0  0) # Request [newer major].0
test_write_basic_config_version_file(4.05     9.1      0  0  0  0) # Request [newer major].[older minor]
test_write_basic_config_version_file(4.05     9.5      0  0  0  0) # Request [newer major].[same minor]
test_write_basic_config_version_file(4.05     9.9      0  0  0  0) # Request [newer major].[newer minor]

test_write_basic_config_version_file(4.05     0.0.0    1  0  0  0) # Request 0.0.0
test_write_basic_config_version_file(4.05     0.0.9    1  0  0  0) # Request 0.0.[newer patch]
test_write_basic_config_version_file(4.05     0.2.0    1  0  0  0) # Request 0.[older minor].0
test_write_basic_config_version_file(4.05     0.2.9    1  0  0  0) # Request 0.[older minor].[newer patch]
test_write_basic_config_version_file(4.05     0.5.0    1  0  0  0) # Request 0.[same minor].0
test_write_basic_config_version_file(4.05     0.5.9    1  0  0  0) # Request 0.[same minor].[newer patch]
test_write_basic_config_version_file(4.05     0.9.0    1  0  0  0) # Request 0.[newer minor].0
test_write_basic_config_version_file(4.05     0.9.9    1  0  0  0) # Request 0.[newer minor].[newer patch]
test_write_basic_config_version_file(4.05     2.0.0    1  0  0  0) # Request [older major].0.0
test_write_basic_config_version_file(4.05     2.0.9    1  0  0  0) # Request [older major].0.[newer patch]
test_write_basic_config_version_file(4.05     2.2.0    1  0  0  0) # Request [older major].[older minor].0
test_write_basic_config_version_file(4.05     2.2.9    1  0  0  0) # Request [older major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05     2.5.0    1  0  0  0) # Request [older major].[same minor].0
test_write_basic_config_version_file(4.05     2.5.9    1  0  0  0) # Request [older major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05     2.9.0    1  0  0  0) # Request [older major].[newer minor].0
test_write_basic_config_version_file(4.05     2.9.9    1  0  0  0) # Request [older major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.05     4.0.0    1  1  0  0) # Request [same major].0.0
test_write_basic_config_version_file(4.05     4.0.9    1  1  0  0) # Request [same major].0.[newer patch]
test_write_basic_config_version_file(4.05     4.2.0    1  1  0  0) # Request [same major].[older minor].0
test_write_basic_config_version_file(4.05     4.2.9    1  1  0  0) # Request [same major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05     4.05.0   1  1  1  0) # Request [same major].[same minor].0
test_write_basic_config_version_file(4.05     4.5.9    0  0  0  0) # Request [same major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05     4.9.0    0  0  0  0) # Request [same major].[newer minor].0
test_write_basic_config_version_file(4.05     4.9.9    0  0  0  0) # Request [same major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.05     9.0.0    0  0  0  0) # Request [newer major].0.0
test_write_basic_config_version_file(4.05     9.0.9    0  0  0  0) # Request [newer major].0.[newer patch]
test_write_basic_config_version_file(4.05     9.2.0    0  0  0  0) # Request [newer major].[older minor].0
test_write_basic_config_version_file(4.05     9.2.9    0  0  0  0) # Request [newer major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05     9.5.0    0  0  0  0) # Request [newer major].[same minor].0
test_write_basic_config_version_file(4.05     9.5.9    0  0  0  0) # Request [newer major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05     9.9.0    0  0  0  0) # Request [newer major].[newer minor].0
test_write_basic_config_version_file(4.05     9.9.9    0  0  0  0) # Request [newer major].[newer minor].[newer patch]

test_write_basic_config_version_file(4.05     0.0.0.0  1  0  0  0) # Request 0.0.0.0
test_write_basic_config_version_file(4.05     0.0.0.9  1  0  0  0) # Request 0.0.0.[newer tweak]
test_write_basic_config_version_file(4.05     0.0.9.0  1  0  0  0) # Request 0.0.[newer patch].0
test_write_basic_config_version_file(4.05     0.0.9.9  1  0  0  0) # Request 0.0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     0.2.0.0  1  0  0  0) # Request 0.[older minor].0.0
test_write_basic_config_version_file(4.05     0.2.0.9  1  0  0  0) # Request 0.[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     0.2.9.0  1  0  0  0) # Request 0.[older minor].[newer patch].0
test_write_basic_config_version_file(4.05     0.2.9.9  1  0  0  0) # Request 0.[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     0.5.0.0  1  0  0  0) # Request 0.[same minor].0.0
test_write_basic_config_version_file(4.05     0.5.0.9  1  0  0  0) # Request 0.[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     0.5.9.0  1  0  0  0) # Request 0.[same minor].[newer patch].0
test_write_basic_config_version_file(4.05     0.5.9.9  1  0  0  0) # Request 0.[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     0.9.0.0  1  0  0  0) # Request 0.[newer minor].0.0
test_write_basic_config_version_file(4.05     0.9.0.9  1  0  0  0) # Request 0.[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     0.9.9.0  1  0  0  0) # Request 0.[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05     0.9.9.9  1  0  0  0) # Request 0.[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     2.0.0.0  1  0  0  0) # Request [older major].0.0.0
test_write_basic_config_version_file(4.05     2.0.0.9  1  0  0  0) # Request [older major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05     2.0.9.0  1  0  0  0) # Request [older major].0.[newer patch].0
test_write_basic_config_version_file(4.05     2.0.9.9  1  0  0  0) # Request [older major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     2.2.0.0  1  0  0  0) # Request [older major].[older minor].0.0
test_write_basic_config_version_file(4.05     2.2.0.9  1  0  0  0) # Request [older major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     2.2.9.0  1  0  0  0) # Request [older major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05     2.2.9.9  1  0  0  0) # Request [older major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     2.5.0.0  1  0  0  0) # Request [older major].[same minor].0.0
test_write_basic_config_version_file(4.05     2.5.0.9  1  0  0  0) # Request [older major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     2.5.9.0  1  0  0  0) # Request [older major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05     2.5.9.9  1  0  0  0) # Request [older major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     2.9.0.0  1  0  0  0) # Request [older major].[newer minor].0.0
test_write_basic_config_version_file(4.05     2.9.0.9  1  0  0  0) # Request [older major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     2.9.9.0  1  0  0  0) # Request [older major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05     2.9.9.9  1  0  0  0) # Request [older major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     4.0.0.0  1  1  0  0) # Request [same major].0.0.0
test_write_basic_config_version_file(4.05     4.0.0.9  1  1  0  0) # Request [same major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05     4.0.9.0  1  1  0  0) # Request [same major].0.[newer patch].0
test_write_basic_config_version_file(4.05     4.0.9.9  1  1  0  0) # Request [same major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     4.2.0.0  1  1  0  0) # Request [same major].[older minor].0.0
test_write_basic_config_version_file(4.05     4.2.0.9  1  1  0  0) # Request [same major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     4.2.9.0  1  1  0  0) # Request [same major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05     4.2.9.9  1  1  0  0) # Request [same major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     4.5.0.0  1  1  1  0) # Request [same major].[same minor].0.0
test_write_basic_config_version_file(4.05     4.5.0.9  0  0  0  0) # Request [same major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     4.5.9.0  0  0  0  0) # Request [same major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05     4.5.9.9  0  0  0  0) # Request [same major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     4.9.0.0  0  0  0  0) # Request [same major].[newer minor].0.0
test_write_basic_config_version_file(4.05     4.9.0.9  0  0  0  0) # Request [same major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     4.9.9.0  0  0  0  0) # Request [same major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05     4.9.9.9  0  0  0  0) # Request [same major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     9.0.0.0  0  0  0  0) # Request [newer major].0.0.0
test_write_basic_config_version_file(4.05     9.0.0.9  0  0  0  0) # Request [newer major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05     9.0.9.0  0  0  0  0) # Request [newer major].0.[newer patch].0
test_write_basic_config_version_file(4.05     9.0.9.9  0  0  0  0) # Request [newer major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     9.2.0.0  0  0  0  0) # Request [newer major].[older minor].0.0
test_write_basic_config_version_file(4.05     9.2.0.9  0  0  0  0) # Request [newer major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     9.2.9.0  0  0  0  0) # Request [newer major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05     9.2.9.9  0  0  0  0) # Request [newer major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     9.5.0.0  0  0  0  0) # Request [newer major].[same minor].0.0
test_write_basic_config_version_file(4.05     9.5.0.9  0  0  0  0) # Request [newer major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     9.5.9.0  0  0  0  0) # Request [newer major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05     9.5.9.9  0  0  0  0) # Request [newer major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05     9.9.0.0  0  0  0  0) # Request [newer major].[newer minor].0.0
test_write_basic_config_version_file(4.05     9.9.0.9  0  0  0  0) # Request [newer major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05     9.9.9.0  0  0  0  0) # Request [newer major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05     9.9.9.9  0  0  0  0) # Request [newer major].[newer minor].[newer patch].[newer tweak]


test_write_basic_config_version_file(4.5.06    0        1  0  0  0) # Request 0
test_write_basic_config_version_file(4.5.06    2        1  0  0  0) # Request [older major]
test_write_basic_config_version_file(4.5.06    4        1  1  0  0) # Request [same major]
test_write_basic_config_version_file(4.5.06    9        0  0  0  0) # Request [newer major]

test_write_basic_config_version_file(4.5.06    0.0      1  0  0  0) # Request 0.0
test_write_basic_config_version_file(4.5.06    0.2      1  0  0  0) # Request 0.[older minor]
test_write_basic_config_version_file(4.5.06    0.5      1  0  0  0) # Request 0.[same minor]
test_write_basic_config_version_file(4.5.06    0.9      1  0  0  0) # Request 0.[newer minor]
test_write_basic_config_version_file(4.5.06    2.0      1  0  0  0) # Request [older major].0
test_write_basic_config_version_file(4.5.06    2.2      1  0  0  0) # Request [older major].[older minor]
test_write_basic_config_version_file(4.5.06    2.5      1  0  0  0) # Request [older major].[same minor]
test_write_basic_config_version_file(4.5.06    2.9      1  0  0  0) # Request [older major].[newer minor]
test_write_basic_config_version_file(4.5.06    4.0      1  1  0  0) # Request [same major].0
test_write_basic_config_version_file(4.5.06    4.2      1  1  0  0) # Request [same major].[older minor]
test_write_basic_config_version_file(4.5.06    4.5      1  1  1  0) # Request [same major].[same minor]
test_write_basic_config_version_file(4.5.06    4.9      0  0  0  0) # Request [same major].[newer minor]
test_write_basic_config_version_file(4.5.06    9.0      0  0  0  0) # Request [newer major].0
test_write_basic_config_version_file(4.5.06    9.1      0  0  0  0) # Request [newer major].[older minor]
test_write_basic_config_version_file(4.5.06    9.5      0  0  0  0) # Request [newer major].[same minor]
test_write_basic_config_version_file(4.5.06    9.9      0  0  0  0) # Request [newer major].[newer minor]

test_write_basic_config_version_file(4.5.06    0.0.0    1  0  0  0) # Request 0.0.0
test_write_basic_config_version_file(4.5.06    0.0.2    1  0  0  0) # Request 0.0.[older patch]
test_write_basic_config_version_file(4.5.06    0.0.6    1  0  0  0) # Request 0.0.[same patch]
test_write_basic_config_version_file(4.5.06    0.0.9    1  0  0  0) # Request 0.0.[newer patch]
test_write_basic_config_version_file(4.5.06    0.2.0    1  0  0  0) # Request 0.[older minor].0
test_write_basic_config_version_file(4.5.06    0.2.2    1  0  0  0) # Request 0.[older minor].[older patch]
test_write_basic_config_version_file(4.5.06    0.2.6    1  0  0  0) # Request 0.[older minor].[same patch]
test_write_basic_config_version_file(4.5.06    0.2.9    1  0  0  0) # Request 0.[older minor].[newer patch]
test_write_basic_config_version_file(4.5.06    0.5.0    1  0  0  0) # Request 0.[same minor].0
test_write_basic_config_version_file(4.5.06    0.5.2    1  0  0  0) # Request 0.[same minor].[older patch]
test_write_basic_config_version_file(4.5.06    0.5.6    1  0  0  0) # Request 0.[same minor].[same patch]
test_write_basic_config_version_file(4.5.06    0.5.9    1  0  0  0) # Request 0.[same minor].[newer patch]
test_write_basic_config_version_file(4.5.06    0.9.0    1  0  0  0) # Request 0.[newer minor].0
test_write_basic_config_version_file(4.5.06    0.9.2    1  0  0  0) # Request 0.[newer minor].[older patch]
test_write_basic_config_version_file(4.5.06    0.9.6    1  0  0  0) # Request 0.[newer minor].[same patch]
test_write_basic_config_version_file(4.5.06    0.9.9    1  0  0  0) # Request 0.[newer minor].[newer patch]
test_write_basic_config_version_file(4.5.06    2.0.0    1  0  0  0) # Request [older major].0.0
test_write_basic_config_version_file(4.5.06    2.0.2    1  0  0  0) # Request [older major].0.[older patch]
test_write_basic_config_version_file(4.5.06    2.0.6    1  0  0  0) # Request [older major].0.[same patch]
test_write_basic_config_version_file(4.5.06    2.0.9    1  0  0  0) # Request [older major].0.[newer patch]
test_write_basic_config_version_file(4.5.06    2.2.0    1  0  0  0) # Request [older major].[older minor].0
test_write_basic_config_version_file(4.5.06    2.2.2    1  0  0  0) # Request [older major].[older minor].[older patch]
test_write_basic_config_version_file(4.5.06    2.2.6    1  0  0  0) # Request [older major].[older minor].[same patch]
test_write_basic_config_version_file(4.5.06    2.2.9    1  0  0  0) # Request [older major].[older minor].[newer patch]
test_write_basic_config_version_file(4.5.06    2.5.0    1  0  0  0) # Request [older major].[same minor].0
test_write_basic_config_version_file(4.5.06    2.5.2    1  0  0  0) # Request [older major].[same minor].[older patch]
test_write_basic_config_version_file(4.5.06    2.5.6    1  0  0  0) # Request [older major].[same minor].[same patch]
test_write_basic_config_version_file(4.5.06    2.5.9    1  0  0  0) # Request [older major].[same minor].[newer patch]
test_write_basic_config_version_file(4.5.06    2.9.0    1  0  0  0) # Request [older major].[newer minor].0
test_write_basic_config_version_file(4.5.06    2.9.2    1  0  0  0) # Request [older major].[newer minor].[older patch]
test_write_basic_config_version_file(4.5.06    2.9.6    1  0  0  0) # Request [older major].[newer minor].[same patch]
test_write_basic_config_version_file(4.5.06    2.9.9    1  0  0  0) # Request [older major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.5.06    4.0.0    1  1  0  0) # Request [same major].0.0
test_write_basic_config_version_file(4.5.06    4.0.2    1  1  0  0) # Request [same major].0.[older patch]
test_write_basic_config_version_file(4.5.06    4.0.6    1  1  0  0) # Request [same major].0.[same patch]
test_write_basic_config_version_file(4.5.06    4.0.9    1  1  0  0) # Request [same major].0.[newer patch]
test_write_basic_config_version_file(4.5.06    4.2.0    1  1  0  0) # Request [same major].[older minor].0
test_write_basic_config_version_file(4.5.06    4.2.2    1  1  0  0) # Request [same major].[older minor].[older patch]
test_write_basic_config_version_file(4.5.06    4.2.6    1  1  0  0) # Request [same major].[older minor].[same patch]
test_write_basic_config_version_file(4.5.06    4.2.9    1  1  0  0) # Request [same major].[older minor].[newer patch]
test_write_basic_config_version_file(4.5.06    4.5.0    1  1  1  0) # Request [same major].[same minor].0
test_write_basic_config_version_file(4.5.06    4.5.2    1  1  1  0) # Request [same major].[same minor].[older patch]
test_write_basic_config_version_file(4.5.06    4.5.06   1  1  1  1) # Request [same major].[same minor].[same patch]
test_write_basic_config_version_file(4.5.06    4.5.9    0  0  0  0) # Request [same major].[same minor].[newer patch]
test_write_basic_config_version_file(4.5.06    4.9.0    0  0  0  0) # Request [same major].[newer minor].0
test_write_basic_config_version_file(4.5.06    4.9.2    0  0  0  0) # Request [same major].[newer minor].[older patch]
test_write_basic_config_version_file(4.5.06    4.9.6    0  0  0  0) # Request [same major].[newer minor].[same patch]
test_write_basic_config_version_file(4.5.06    4.9.9    0  0  0  0) # Request [same major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.5.06    9.0.0    0  0  0  0) # Request [newer major].0.0
test_write_basic_config_version_file(4.5.06    9.0.2    0  0  0  0) # Request [newer major].0.[older patch]
test_write_basic_config_version_file(4.5.06    9.0.6    0  0  0  0) # Request [newer major].0.[same patch]
test_write_basic_config_version_file(4.5.06    9.0.9    0  0  0  0) # Request [newer major].0.[newer patch]
test_write_basic_config_version_file(4.5.06    9.2.0    0  0  0  0) # Request [newer major].[older minor].0
test_write_basic_config_version_file(4.5.06    9.2.2    0  0  0  0) # Request [newer major].[older minor].[older patch]
test_write_basic_config_version_file(4.5.06    9.2.6    0  0  0  0) # Request [newer major].[older minor].[same patch]
test_write_basic_config_version_file(4.5.06    9.2.9    0  0  0  0) # Request [newer major].[older minor].[newer patch]
test_write_basic_config_version_file(4.5.06    9.5.0    0  0  0  0) # Request [newer major].[same minor].0
test_write_basic_config_version_file(4.5.06    9.5.2    0  0  0  0) # Request [newer major].[same minor].[older patch]
test_write_basic_config_version_file(4.5.06    9.5.6    0  0  0  0) # Request [newer major].[same minor].[same patch]
test_write_basic_config_version_file(4.5.06    9.5.9    0  0  0  0) # Request [newer major].[same minor].[newer patch]
test_write_basic_config_version_file(4.5.06    9.9.0    0  0  0  0) # Request [newer major].[newer minor].0
test_write_basic_config_version_file(4.5.06    9.9.2    0  0  0  0) # Request [newer major].[newer minor].[older patch]
test_write_basic_config_version_file(4.5.06    9.9.6    0  0  0  0) # Request [newer major].[newer minor].[same patch]
test_write_basic_config_version_file(4.5.06    9.9.9    0  0  0  0) # Request [newer major].[newer minor].[newer patch]

test_write_basic_config_version_file(4.5.06    0.0.0.0  1  0  0  0) # Request 0.0.0.0
test_write_basic_config_version_file(4.5.06    0.0.0.9  1  0  0  0) # Request 0.0.0.[newer tweak]
test_write_basic_config_version_file(4.5.06    0.0.2.0  1  0  0  0) # Request 0.0.[older patch].0
test_write_basic_config_version_file(4.5.06    0.0.2.9  1  0  0  0) # Request 0.0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.0.6.0  1  0  0  0) # Request 0.0.[same patch].0
test_write_basic_config_version_file(4.5.06    0.0.6.9  1  0  0  0) # Request 0.0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.0.9.0  1  0  0  0) # Request 0.0.[newer patch].0
test_write_basic_config_version_file(4.5.06    0.0.9.9  1  0  0  0) # Request 0.0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.2.0.0  1  0  0  0) # Request 0.[older minor].0.0
test_write_basic_config_version_file(4.5.06    0.2.0.9  1  0  0  0) # Request 0.[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    0.2.2.0  1  0  0  0) # Request 0.[older minor].[older patch].0
test_write_basic_config_version_file(4.5.06    0.2.2.9  1  0  0  0) # Request 0.[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.2.6.0  1  0  0  0) # Request 0.[older minor].[same patch].0
test_write_basic_config_version_file(4.5.06    0.2.6.9  1  0  0  0) # Request 0.[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.2.9.0  1  0  0  0) # Request 0.[older minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    0.2.9.9  1  0  0  0) # Request 0.[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.5.0.0  1  0  0  0) # Request 0.[same minor].0.0
test_write_basic_config_version_file(4.5.06    0.5.0.9  1  0  0  0) # Request 0.[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    0.5.2.0  1  0  0  0) # Request 0.[same minor].[older patch].0
test_write_basic_config_version_file(4.5.06    0.5.2.9  1  0  0  0) # Request 0.[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.5.6.0  1  0  0  0) # Request 0.[same minor].[same patch].0
test_write_basic_config_version_file(4.5.06    0.5.6.9  1  0  0  0) # Request 0.[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.5.9.0  1  0  0  0) # Request 0.[same minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    0.5.9.9  1  0  0  0) # Request 0.[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.9.0.0  1  0  0  0) # Request 0.[newer minor].0.0
test_write_basic_config_version_file(4.5.06    0.9.0.9  1  0  0  0) # Request 0.[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    0.9.2.0  1  0  0  0) # Request 0.[newer minor].[older patch].0
test_write_basic_config_version_file(4.5.06    0.9.2.9  1  0  0  0) # Request 0.[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.9.6.0  1  0  0  0) # Request 0.[newer minor].[same patch].0
test_write_basic_config_version_file(4.5.06    0.9.6.9  1  0  0  0) # Request 0.[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    0.9.9.0  1  0  0  0) # Request 0.[newer minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    0.9.9.9  1  0  0  0) # Request 0.[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.0.0.0  1  0  0  0) # Request [older major].0.0.0
test_write_basic_config_version_file(4.5.06    2.0.0.9  1  0  0  0) # Request [older major].0.0.[newer tweak]
test_write_basic_config_version_file(4.5.06    2.0.2.0  1  0  0  0) # Request [older major].0.[older patch].0
test_write_basic_config_version_file(4.5.06    2.0.2.9  1  0  0  0) # Request [older major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.0.6.0  1  0  0  0) # Request [older major].0.[same patch].0
test_write_basic_config_version_file(4.5.06    2.0.6.9  1  0  0  0) # Request [older major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.0.9.0  1  0  0  0) # Request [older major].0.[newer patch].0
test_write_basic_config_version_file(4.5.06    2.0.9.9  1  0  0  0) # Request [older major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.2.0.0  1  0  0  0) # Request [older major].[older minor].0.0
test_write_basic_config_version_file(4.5.06    2.2.0.9  1  0  0  0) # Request [older major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    2.2.2.0  1  0  0  0) # Request [older major].[older minor].[older patch].0
test_write_basic_config_version_file(4.5.06    2.2.2.9  1  0  0  0) # Request [older major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.2.6.0  1  0  0  0) # Request [older major].[older minor].[same patch].0
test_write_basic_config_version_file(4.5.06    2.2.6.9  1  0  0  0) # Request [older major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.2.9.0  1  0  0  0) # Request [older major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    2.2.9.9  1  0  0  0) # Request [older major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.5.0.0  1  0  0  0) # Request [older major].[same minor].0.0
test_write_basic_config_version_file(4.5.06    2.5.0.9  1  0  0  0) # Request [older major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    2.5.2.0  1  0  0  0) # Request [older major].[same minor].[older patch].0
test_write_basic_config_version_file(4.5.06    2.5.2.9  1  0  0  0) # Request [older major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.5.6.0  1  0  0  0) # Request [older major].[same minor].[same patch].0
test_write_basic_config_version_file(4.5.06    2.5.6.9  1  0  0  0) # Request [older major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.5.9.0  1  0  0  0) # Request [older major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    2.5.9.9  1  0  0  0) # Request [older major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.9.0.0  1  0  0  0) # Request [older major].[newer minor].0.0
test_write_basic_config_version_file(4.5.06    2.9.0.9  1  0  0  0) # Request [older major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    2.9.2.0  1  0  0  0) # Request [older major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.5.06    2.9.2.9  1  0  0  0) # Request [older major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.9.6.0  1  0  0  0) # Request [older major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.5.06    2.9.6.9  1  0  0  0) # Request [older major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    2.9.9.0  1  0  0  0) # Request [older major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    2.9.9.9  1  0  0  0) # Request [older major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.0.0.0  1  1  0  0) # Request [same major].0.0.0
test_write_basic_config_version_file(4.5.06    4.0.0.9  1  1  0  0) # Request [same major].0.0.[newer tweak]
test_write_basic_config_version_file(4.5.06    4.0.2.0  1  1  0  0) # Request [same major].0.[older patch].0
test_write_basic_config_version_file(4.5.06    4.0.2.9  1  1  0  0) # Request [same major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.0.6.0  1  1  0  0) # Request [same major].0.[same patch].0
test_write_basic_config_version_file(4.5.06    4.0.6.9  1  1  0  0) # Request [same major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.0.9.0  1  1  0  0) # Request [same major].0.[newer patch].0
test_write_basic_config_version_file(4.5.06    4.0.9.9  1  1  0  0) # Request [same major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.2.0.0  1  1  0  0) # Request [same major].[older minor].0.0
test_write_basic_config_version_file(4.5.06    4.2.0.9  1  1  0  0) # Request [same major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    4.2.2.0  1  1  0  0) # Request [same major].[older minor].[older patch].0
test_write_basic_config_version_file(4.5.06    4.2.2.9  1  1  0  0) # Request [same major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.2.6.0  1  1  0  0) # Request [same major].[older minor].[same patch].0
test_write_basic_config_version_file(4.5.06    4.2.6.9  1  1  0  0) # Request [same major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.2.9.0  1  1  0  0) # Request [same major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    4.2.9.9  1  1  0  0) # Request [same major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.5.0.0  1  1  1  0) # Request [same major].[same minor].0.0
test_write_basic_config_version_file(4.5.06    4.5.0.9  1  1  1  0) # Request [same major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    4.5.2.0  1  1  1  0) # Request [same major].[same minor].[older patch].0
test_write_basic_config_version_file(4.5.06    4.5.2.9  1  1  1  0) # Request [same major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.5.06.0 1  1  1  1) # Request [same major].[same minor].[same patch].0
test_write_basic_config_version_file(4.5.06    4.5.06.9 0  0  0  1) # Request [same major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.5.9.0  0  0  0  0) # Request [same major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    4.5.9.9  0  0  0  0) # Request [same major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.9.0.0  0  0  0  0) # Request [same major].[newer minor].0.0
test_write_basic_config_version_file(4.5.06    4.9.0.9  0  0  0  0) # Request [same major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    4.9.2.0  0  0  0  0) # Request [same major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.5.06    4.9.2.9  0  0  0  0) # Request [same major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.9.6.0  0  0  0  0) # Request [same major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.5.06    4.9.6.9  0  0  0  0) # Request [same major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    4.9.9.0  0  0  0  0) # Request [same major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    4.9.9.9  0  0  0  0) # Request [same major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.0.0.0  0  0  0  0) # Request [newer major].0.0.0
test_write_basic_config_version_file(4.5.06    9.0.0.9  0  0  0  0) # Request [newer major].0.0.[newer tweak]
test_write_basic_config_version_file(4.5.06    9.0.2.0  0  0  0  0) # Request [newer major].0.[older patch].0
test_write_basic_config_version_file(4.5.06    9.0.2.9  0  0  0  0) # Request [newer major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.0.6.0  0  0  0  0) # Request [newer major].0.[same patch].0
test_write_basic_config_version_file(4.5.06    9.0.6.9  0  0  0  0) # Request [newer major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.0.9.0  0  0  0  0) # Request [newer major].0.[newer patch].0
test_write_basic_config_version_file(4.5.06    9.0.9.9  0  0  0  0) # Request [newer major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.2.0.0  0  0  0  0) # Request [newer major].[older minor].0.0
test_write_basic_config_version_file(4.5.06    9.2.0.9  0  0  0  0) # Request [newer major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    9.2.2.0  0  0  0  0) # Request [newer major].[older minor].[older patch].0
test_write_basic_config_version_file(4.5.06    9.2.2.9  0  0  0  0) # Request [newer major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.2.6.0  0  0  0  0) # Request [newer major].[older minor].[same patch].0
test_write_basic_config_version_file(4.5.06    9.2.6.9  0  0  0  0) # Request [newer major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.2.9.0  0  0  0  0) # Request [newer major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    9.2.9.9  0  0  0  0) # Request [newer major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.5.0.0  0  0  0  0) # Request [newer major].[same minor].0.0
test_write_basic_config_version_file(4.5.06    9.5.0.9  0  0  0  0) # Request [newer major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    9.5.2.0  0  0  0  0) # Request [newer major].[same minor].[older patch].0
test_write_basic_config_version_file(4.5.06    9.5.2.9  0  0  0  0) # Request [newer major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.5.6.0  0  0  0  0) # Request [newer major].[same minor].[same patch].0
test_write_basic_config_version_file(4.5.06    9.5.6.9  0  0  0  0) # Request [newer major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.5.9.0  0  0  0  0) # Request [newer major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    9.5.9.9  0  0  0  0) # Request [newer major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.9.0.0  0  0  0  0) # Request [newer major].[newer minor].0.0
test_write_basic_config_version_file(4.5.06    9.9.0.9  0  0  0  0) # Request [newer major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.5.06    9.9.2.0  0  0  0  0) # Request [newer major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.5.06    9.9.2.9  0  0  0  0) # Request [newer major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.9.6.0  0  0  0  0) # Request [newer major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.5.06    9.9.6.9  0  0  0  0) # Request [newer major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.5.06    9.9.9.0  0  0  0  0) # Request [newer major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.5.06    9.9.9.9  0  0  0  0) # Request [newer major].[newer minor].[newer patch].[newer tweak]


test_write_basic_config_version_file(4.05.06.007  0        1  0  0  0) # Request 0
test_write_basic_config_version_file(4.05.06.007  2        1  0  0  0) # Request [older major]
test_write_basic_config_version_file(4.05.06.007  4        1  1  0  0) # Request [same major]
test_write_basic_config_version_file(4.05.06.007  9        0  0  0  0) # Request [newer major]

test_write_basic_config_version_file(4.05.06.007  0.0      1  0  0  0) # Request 0.0
test_write_basic_config_version_file(4.05.06.007  0.2      1  0  0  0) # Request 0.[older minor]
test_write_basic_config_version_file(4.05.06.007  0.5      1  0  0  0) # Request 0.[same minor]
test_write_basic_config_version_file(4.05.06.007  0.9      1  0  0  0) # Request 0.[newer minor]
test_write_basic_config_version_file(4.05.06.007  2.0      1  0  0  0) # Request [older major].0
test_write_basic_config_version_file(4.05.06.007  2.2      1  0  0  0) # Request [older major].[older minor]
test_write_basic_config_version_file(4.05.06.007  2.5      1  0  0  0) # Request [older major].[same minor]
test_write_basic_config_version_file(4.05.06.007  2.9      1  0  0  0) # Request [older major].[newer minor]
test_write_basic_config_version_file(4.05.06.007  4.0      1  1  0  0) # Request [same major].0
test_write_basic_config_version_file(4.05.06.007  4.2      1  1  0  0) # Request [same major].[older minor]
test_write_basic_config_version_file(4.05.06.007  4.5      1  1  1  0) # Request [same major].[same minor]
test_write_basic_config_version_file(4.05.06.007  4.9      0  0  0  0) # Request [same major].[newer minor]
test_write_basic_config_version_file(4.05.06.007  9.0      0  0  0  0) # Request [newer major].0
test_write_basic_config_version_file(4.05.06.007  9.1      0  0  0  0) # Request [newer major].[older minor]
test_write_basic_config_version_file(4.05.06.007  9.5      0  0  0  0) # Request [newer major].[same minor]
test_write_basic_config_version_file(4.05.06.007  9.9      0  0  0  0) # Request [newer major].[newer minor]

test_write_basic_config_version_file(4.05.06.007  0.0.0    1  0  0  0) # Request 0.0.0
test_write_basic_config_version_file(4.05.06.007  0.0.2    1  0  0  0) # Request 0.0.[older patch]
test_write_basic_config_version_file(4.05.06.007  0.0.6    1  0  0  0) # Request 0.0.[same patch]
test_write_basic_config_version_file(4.05.06.007  0.0.9    1  0  0  0) # Request 0.0.[newer patch]
test_write_basic_config_version_file(4.05.06.007  0.2.0    1  0  0  0) # Request 0.[older minor].0
test_write_basic_config_version_file(4.05.06.007  0.2.2    1  0  0  0) # Request 0.[older minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  0.2.6    1  0  0  0) # Request 0.[older minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  0.2.9    1  0  0  0) # Request 0.[older minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  0.5.0    1  0  0  0) # Request 0.[same minor].0
test_write_basic_config_version_file(4.05.06.007  0.5.2    1  0  0  0) # Request 0.[same minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  0.5.6    1  0  0  0) # Request 0.[same minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  0.5.9    1  0  0  0) # Request 0.[same minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  0.9.0    1  0  0  0) # Request 0.[newer minor].0
test_write_basic_config_version_file(4.05.06.007  0.9.2    1  0  0  0) # Request 0.[newer minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  0.9.6    1  0  0  0) # Request 0.[newer minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  0.9.9    1  0  0  0) # Request 0.[newer minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  2.0.0    1  0  0  0) # Request [older major].0.0
test_write_basic_config_version_file(4.05.06.007  2.0.2    1  0  0  0) # Request [older major].0.[older patch]
test_write_basic_config_version_file(4.05.06.007  2.0.6    1  0  0  0) # Request [older major].0.[same patch]
test_write_basic_config_version_file(4.05.06.007  2.0.9    1  0  0  0) # Request [older major].0.[newer patch]
test_write_basic_config_version_file(4.05.06.007  2.2.0    1  0  0  0) # Request [older major].[older minor].0
test_write_basic_config_version_file(4.05.06.007  2.2.2    1  0  0  0) # Request [older major].[older minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  2.2.6    1  0  0  0) # Request [older major].[older minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  2.2.9    1  0  0  0) # Request [older major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  2.5.0    1  0  0  0) # Request [older major].[same minor].0
test_write_basic_config_version_file(4.05.06.007  2.5.2    1  0  0  0) # Request [older major].[same minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  2.5.6    1  0  0  0) # Request [older major].[same minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  2.5.9    1  0  0  0) # Request [older major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  2.9.0    1  0  0  0) # Request [older major].[newer minor].0
test_write_basic_config_version_file(4.05.06.007  2.9.2    1  0  0  0) # Request [older major].[newer minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  2.9.6    1  0  0  0) # Request [older major].[newer minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  2.9.9    1  0  0  0) # Request [older major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  4.0.0    1  1  0  0) # Request [same major].0.0
test_write_basic_config_version_file(4.05.06.007  4.0.2    1  1  0  0) # Request [same major].0.[older patch]
test_write_basic_config_version_file(4.05.06.007  4.0.6    1  1  0  0) # Request [same major].0.[same patch]
test_write_basic_config_version_file(4.05.06.007  4.0.9    1  1  0  0) # Request [same major].0.[newer patch]
test_write_basic_config_version_file(4.05.06.007  4.2.0    1  1  0  0) # Request [same major].[older minor].0
test_write_basic_config_version_file(4.05.06.007  4.2.2    1  1  0  0) # Request [same major].[older minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  4.2.6    1  1  0  0) # Request [same major].[older minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  4.2.9    1  1  0  0) # Request [same major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  4.5.0    1  1  1  0) # Request [same major].[same minor].0
test_write_basic_config_version_file(4.05.06.007  4.5.2    1  1  1  0) # Request [same major].[same minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  4.05.06  1  1  1  1) # Request [same major].[same minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  4.5.9    0  0  0  0) # Request [same major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  4.9.0    0  0  0  0) # Request [same major].[newer minor].0
test_write_basic_config_version_file(4.05.06.007  4.9.2    0  0  0  0) # Request [same major].[newer minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  4.9.6    0  0  0  0) # Request [same major].[newer minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  4.9.9    0  0  0  0) # Request [same major].[newer minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  9.0.0    0  0  0  0) # Request [newer major].0.0
test_write_basic_config_version_file(4.05.06.007  9.0.2    0  0  0  0) # Request [newer major].0.[older patch]
test_write_basic_config_version_file(4.05.06.007  9.0.6    0  0  0  0) # Request [newer major].0.[same patch]
test_write_basic_config_version_file(4.05.06.007  9.0.9    0  0  0  0) # Request [newer major].0.[newer patch]
test_write_basic_config_version_file(4.05.06.007  9.2.0    0  0  0  0) # Request [newer major].[older minor].0
test_write_basic_config_version_file(4.05.06.007  9.2.2    0  0  0  0) # Request [newer major].[older minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  9.2.6    0  0  0  0) # Request [newer major].[older minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  9.2.9    0  0  0  0) # Request [newer major].[older minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  9.5.0    0  0  0  0) # Request [newer major].[same minor].0
test_write_basic_config_version_file(4.05.06.007  9.5.2    0  0  0  0) # Request [newer major].[same minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  9.5.6    0  0  0  0) # Request [newer major].[same minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  9.5.9    0  0  0  0) # Request [newer major].[same minor].[newer patch]
test_write_basic_config_version_file(4.05.06.007  9.9.0    0  0  0  0) # Request [newer major].[newer minor].0
test_write_basic_config_version_file(4.05.06.007  9.9.2    0  0  0  0) # Request [newer major].[newer minor].[older patch]
test_write_basic_config_version_file(4.05.06.007  9.9.6    0  0  0  0) # Request [newer major].[newer minor].[same patch]
test_write_basic_config_version_file(4.05.06.007  9.9.9    0  0  0  0) # Request [newer major].[newer minor].[newer patch]

test_write_basic_config_version_file(4.05.06.007  0.0.0.0  1  0  0  0) # Request 0.0.0.0
test_write_basic_config_version_file(4.05.06.007  0.0.0.2  1  0  0  0) # Request 0.0.0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.0.7  1  0  0  0) # Request 0.0.0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.0.9  1  0  0  0) # Request 0.0.0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.2.0  1  0  0  0) # Request 0.0.[older patch].0
test_write_basic_config_version_file(4.05.06.007  0.0.2.2  1  0  0  0) # Request 0.0.[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.2.7  1  0  0  0) # Request 0.0.[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.2.9  1  0  0  0) # Request 0.0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.6.0  1  0  0  0) # Request 0.0.[same patch].0
test_write_basic_config_version_file(4.05.06.007  0.0.6.2  1  0  0  0) # Request 0.0.[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.6.7  1  0  0  0) # Request 0.0.[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.6.9  1  0  0  0) # Request 0.0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.9.0  1  0  0  0) # Request 0.0.[newer patch].0
test_write_basic_config_version_file(4.05.06.007  0.0.9.2  1  0  0  0) # Request 0.0.[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.9.7  1  0  0  0) # Request 0.0.[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.0.9.9  1  0  0  0) # Request 0.0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.0.0  1  0  0  0) # Request 0.[older minor].0.0
test_write_basic_config_version_file(4.05.06.007  0.2.0.2  1  0  0  0) # Request 0.[older minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.0.7  1  0  0  0) # Request 0.[older minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.0.9  1  0  0  0) # Request 0.[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.2.0  1  0  0  0) # Request 0.[older minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  0.2.2.2  1  0  0  0) # Request 0.[older minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.2.7  1  0  0  0) # Request 0.[older minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.2.9  1  0  0  0) # Request 0.[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.6.0  1  0  0  0) # Request 0.[older minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  0.2.6.2  1  0  0  0) # Request 0.[older minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.6.7  1  0  0  0) # Request 0.[older minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.6.9  1  0  0  0) # Request 0.[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.9.0  1  0  0  0) # Request 0.[older minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  0.2.9.2  1  0  0  0) # Request 0.[older minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.9.7  1  0  0  0) # Request 0.[older minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.2.9.9  1  0  0  0) # Request 0.[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.0.0  1  0  0  0) # Request 0.[same minor].0.0
test_write_basic_config_version_file(4.05.06.007  0.5.0.2  1  0  0  0) # Request 0.[same minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.0.7  1  0  0  0) # Request 0.[same minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.0.9  1  0  0  0) # Request 0.[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.2.0  1  0  0  0) # Request 0.[same minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  0.5.2.2  1  0  0  0) # Request 0.[same minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.2.7  1  0  0  0) # Request 0.[same minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.2.9  1  0  0  0) # Request 0.[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.6.0  1  0  0  0) # Request 0.[same minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  0.5.6.2  1  0  0  0) # Request 0.[same minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.6.7  1  0  0  0) # Request 0.[same minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.6.9  1  0  0  0) # Request 0.[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.9.0  1  0  0  0) # Request 0.[same minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  0.5.9.2  1  0  0  0) # Request 0.[same minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.9.7  1  0  0  0) # Request 0.[same minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.5.9.9  1  0  0  0) # Request 0.[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.0.0  1  0  0  0) # Request 0.[newer minor].0.0
test_write_basic_config_version_file(4.05.06.007  0.9.0.2  1  0  0  0) # Request 0.[newer minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.0.7  1  0  0  0) # Request 0.[newer minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.0.9  1  0  0  0) # Request 0.[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.2.0  1  0  0  0) # Request 0.[newer minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  0.9.2.2  1  0  0  0) # Request 0.[newer minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.2.7  1  0  0  0) # Request 0.[newer minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.2.9  1  0  0  0) # Request 0.[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.6.0  1  0  0  0) # Request 0.[newer minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  0.9.6.2  1  0  0  0) # Request 0.[newer minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.6.7  1  0  0  0) # Request 0.[newer minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.6.9  1  0  0  0) # Request 0.[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.9.0  1  0  0  0) # Request 0.[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  0.9.9.2  1  0  0  0) # Request 0.[newer minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.9.7  1  0  0  0) # Request 0.[newer minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  0.9.9.9  1  0  0  0) # Request 0.[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.0.0  1  0  0  0) # Request [older major].0.0.0
test_write_basic_config_version_file(4.05.06.007  2.0.0.2  1  0  0  0) # Request [older major].0.0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.0.7  1  0  0  0) # Request [older major].0.0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.0.9  1  0  0  0) # Request [older major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.2.0  1  0  0  0) # Request [older major].0.[older patch].0
test_write_basic_config_version_file(4.05.06.007  2.0.2.2  1  0  0  0) # Request [older major].0.[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.2.7  1  0  0  0) # Request [older major].0.[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.2.9  1  0  0  0) # Request [older major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.6.0  1  0  0  0) # Request [older major].0.[same patch].0
test_write_basic_config_version_file(4.05.06.007  2.0.6.2  1  0  0  0) # Request [older major].0.[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.6.7  1  0  0  0) # Request [older major].0.[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.6.9  1  0  0  0) # Request [older major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.9.0  1  0  0  0) # Request [older major].0.[newer patch].0
test_write_basic_config_version_file(4.05.06.007  2.0.9.2  1  0  0  0) # Request [older major].0.[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.9.7  1  0  0  0) # Request [older major].0.[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.0.9.9  1  0  0  0) # Request [older major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.0.0  1  0  0  0) # Request [older major].[older minor].0.0
test_write_basic_config_version_file(4.05.06.007  2.2.0.2  1  0  0  0) # Request [older major].[older minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.0.7  1  0  0  0) # Request [older major].[older minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.0.9  1  0  0  0) # Request [older major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.2.0  1  0  0  0) # Request [older major].[older minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  2.2.2.2  1  0  0  0) # Request [older major].[older minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.2.7  1  0  0  0) # Request [older major].[older minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.2.9  1  0  0  0) # Request [older major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.6.0  1  0  0  0) # Request [older major].[older minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  2.2.6.2  1  0  0  0) # Request [older major].[older minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.6.7  1  0  0  0) # Request [older major].[older minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.6.9  1  0  0  0) # Request [older major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.9.0  1  0  0  0) # Request [older major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  2.2.9.2  1  0  0  0) # Request [older major].[older minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.9.7  1  0  0  0) # Request [older major].[older minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.2.9.9  1  0  0  0) # Request [older major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.0.0  1  0  0  0) # Request [older major].[same minor].0.0
test_write_basic_config_version_file(4.05.06.007  2.5.0.2  1  0  0  0) # Request [older major].[same minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.0.7  1  0  0  0) # Request [older major].[same minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.0.9  1  0  0  0) # Request [older major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.2.0  1  0  0  0) # Request [older major].[same minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  2.5.2.2  1  0  0  0) # Request [older major].[same minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.2.7  1  0  0  0) # Request [older major].[same minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.2.9  1  0  0  0) # Request [older major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.6.0  1  0  0  0) # Request [older major].[same minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  2.5.6.2  1  0  0  0) # Request [older major].[same minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.6.7  1  0  0  0) # Request [older major].[same minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.6.9  1  0  0  0) # Request [older major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.9.0  1  0  0  0) # Request [older major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  2.5.9.2  1  0  0  0) # Request [older major].[same minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.9.7  1  0  0  0) # Request [older major].[same minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.5.9.9  1  0  0  0) # Request [older major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.0.0  1  0  0  0) # Request [older major].[newer minor].0.0
test_write_basic_config_version_file(4.05.06.007  2.9.0.2  1  0  0  0) # Request [older major].[newer minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.0.7  1  0  0  0) # Request [older major].[newer minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.0.9  1  0  0  0) # Request [older major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.2.0  1  0  0  0) # Request [older major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  2.9.2.2  1  0  0  0) # Request [older major].[newer minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.2.7  1  0  0  0) # Request [older major].[newer minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.2.9  1  0  0  0) # Request [older major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.6.0  1  0  0  0) # Request [older major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  2.9.6.2  1  0  0  0) # Request [older major].[newer minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.6.7  1  0  0  0) # Request [older major].[newer minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.6.9  1  0  0  0) # Request [older major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.9.0  1  0  0  0) # Request [older major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  2.9.9.2  1  0  0  0) # Request [older major].[newer minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.9.7  1  0  0  0) # Request [older major].[newer minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  2.9.9.9  1  0  0  0) # Request [older major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.0.0  1  1  0  0) # Request [same major].0.0.0
test_write_basic_config_version_file(4.05.06.007  4.0.0.2  1  1  0  0) # Request [same major].0.0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.0.7  1  1  0  0) # Request [same major].0.0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.0.9  1  1  0  0) # Request [same major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.2.0  1  1  0  0) # Request [same major].0.[older patch].0
test_write_basic_config_version_file(4.05.06.007  4.0.2.2  1  1  0  0) # Request [same major].0.[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.2.7  1  1  0  0) # Request [same major].0.[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.2.9  1  1  0  0) # Request [same major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.6.0  1  1  0  0) # Request [same major].0.[same patch].0
test_write_basic_config_version_file(4.05.06.007  4.0.6.2  1  1  0  0) # Request [same major].0.[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.6.7  1  1  0  0) # Request [same major].0.[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.6.9  1  1  0  0) # Request [same major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.9.0  1  1  0  0) # Request [same major].0.[newer patch].0
test_write_basic_config_version_file(4.05.06.007  4.0.9.2  1  1  0  0) # Request [same major].0.[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.9.7  1  1  0  0) # Request [same major].0.[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.0.9.9  1  1  0  0) # Request [same major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.0.0  1  1  0  0) # Request [same major].[older minor].0.0
test_write_basic_config_version_file(4.05.06.007  4.2.0.2  1  1  0  0) # Request [same major].[older minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.0.7  1  1  0  0) # Request [same major].[older minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.0.9  1  1  0  0) # Request [same major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.2.0  1  1  0  0) # Request [same major].[older minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  4.2.2.2  1  1  0  0) # Request [same major].[older minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.2.7  1  1  0  0) # Request [same major].[older minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.2.9  1  1  0  0) # Request [same major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.6.0  1  1  0  0) # Request [same major].[older minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  4.2.6.2  1  1  0  0) # Request [same major].[older minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.6.7  1  1  0  0) # Request [same major].[older minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.6.9  1  1  0  0) # Request [same major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.9.0  1  1  0  0) # Request [same major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  4.2.9.2  1  1  0  0) # Request [same major].[older minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.9.7  1  1  0  0) # Request [same major].[older minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.2.9.9  1  1  0  0) # Request [same major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.0.0  1  1  1  0) # Request [same major].[same minor].0.0
test_write_basic_config_version_file(4.05.06.007  4.5.0.2  1  1  1  0) # Request [same major].[same minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.0.7  1  1  1  0) # Request [same major].[same minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.0.9  1  1  1  0) # Request [same major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.2.0  1  1  1  0) # Request [same major].[same minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  4.5.2.2  1  1  1  0) # Request [same major].[same minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.2.7  1  1  1  0) # Request [same major].[same minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.2.9  1  1  1  0) # Request [same major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.05.06.007 1  1  1  1) # Request [same major].[same minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  4.05.06.2   1  1  1  1) # Request [same major].[same minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.05.06.7   1  1  1  1) # Request [same major].[same minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.05.06.9   0  0  0  1) # Request [same major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.9.0  0  0  0  0) # Request [same major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  4.5.9.2  0  0  0  0) # Request [same major].[same minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.9.7  0  0  0  0) # Request [same major].[same minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.5.9.9  0  0  0  0) # Request [same major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.0.0  0  0  0  0) # Request [same major].[newer minor].0.0
test_write_basic_config_version_file(4.05.06.007  4.9.0.2  0  0  0  0) # Request [same major].[newer minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.0.7  0  0  0  0) # Request [same major].[newer minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.0.9  0  0  0  0) # Request [same major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.2.0  0  0  0  0) # Request [same major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  4.9.2.2  0  0  0  0) # Request [same major].[newer minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.2.7  0  0  0  0) # Request [same major].[newer minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.2.9  0  0  0  0) # Request [same major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.6.0  0  0  0  0) # Request [same major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  4.9.6.2  0  0  0  0) # Request [same major].[newer minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.6.7  0  0  0  0) # Request [same major].[newer minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.6.9  0  0  0  0) # Request [same major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.9.0  0  0  0  0) # Request [same major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  4.9.9.2  0  0  0  0) # Request [same major].[newer minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.9.7  0  0  0  0) # Request [same major].[newer minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  4.9.9.9  0  0  0  0) # Request [same major].[newer minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.0.0  0  0  0  0) # Request [newer major].0.0.0
test_write_basic_config_version_file(4.05.06.007  9.0.0.2  0  0  0  0) # Request [newer major].0.0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.0.7  0  0  0  0) # Request [newer major].0.0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.0.9  0  0  0  0) # Request [newer major].0.0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.2.0  0  0  0  0) # Request [newer major].0.[older patch].0
test_write_basic_config_version_file(4.05.06.007  9.0.2.2  0  0  0  0) # Request [newer major].0.[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.2.7  0  0  0  0) # Request [newer major].0.[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.2.9  0  0  0  0) # Request [newer major].0.[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.6.0  0  0  0  0) # Request [newer major].0.[same patch].0
test_write_basic_config_version_file(4.05.06.007  9.0.6.2  0  0  0  0) # Request [newer major].0.[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.6.7  0  0  0  0) # Request [newer major].0.[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.6.9  0  0  0  0) # Request [newer major].0.[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.9.0  0  0  0  0) # Request [newer major].0.[newer patch].0
test_write_basic_config_version_file(4.05.06.007  9.0.9.2  0  0  0  0) # Request [newer major].0.[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.9.7  0  0  0  0) # Request [newer major].0.[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.0.9.9  0  0  0  0) # Request [newer major].0.[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.0.0  0  0  0  0) # Request [newer major].[older minor].0.0
test_write_basic_config_version_file(4.05.06.007  9.2.0.2  0  0  0  0) # Request [newer major].[older minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.0.7  0  0  0  0) # Request [newer major].[older minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.0.9  0  0  0  0) # Request [newer major].[older minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.2.0  0  0  0  0) # Request [newer major].[older minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  9.2.2.2  0  0  0  0) # Request [newer major].[older minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.2.7  0  0  0  0) # Request [newer major].[older minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.2.9  0  0  0  0) # Request [newer major].[older minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.6.0  0  0  0  0) # Request [newer major].[older minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  9.2.6.2  0  0  0  0) # Request [newer major].[older minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.6.7  0  0  0  0) # Request [newer major].[older minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.6.9  0  0  0  0) # Request [newer major].[older minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.9.0  0  0  0  0) # Request [newer major].[older minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  9.2.9.2  0  0  0  0) # Request [newer major].[older minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.9.7  0  0  0  0) # Request [newer major].[older minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.2.9.9  0  0  0  0) # Request [newer major].[older minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.0.0  0  0  0  0) # Request [newer major].[same minor].0.0
test_write_basic_config_version_file(4.05.06.007  9.5.0.2  0  0  0  0) # Request [newer major].[same minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.0.7  0  0  0  0) # Request [newer major].[same minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.0.9  0  0  0  0) # Request [newer major].[same minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.2.0  0  0  0  0) # Request [newer major].[same minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  9.5.2.2  0  0  0  0) # Request [newer major].[same minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.2.7  0  0  0  0) # Request [newer major].[same minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.2.9  0  0  0  0) # Request [newer major].[same minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.6.0  0  0  0  0) # Request [newer major].[same minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  9.5.6.2  0  0  0  0) # Request [newer major].[same minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.6.7  0  0  0  0) # Request [newer major].[same minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.6.9  0  0  0  0) # Request [newer major].[same minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.9.0  0  0  0  0) # Request [newer major].[same minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  9.5.9.2  0  0  0  0) # Request [newer major].[same minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.9.7  0  0  0  0) # Request [newer major].[same minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.5.9.9  0  0  0  0) # Request [newer major].[same minor].[newer patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.0.0  0  0  0  0) # Request [newer major].[newer minor].0.0
test_write_basic_config_version_file(4.05.06.007  9.9.0.2  0  0  0  0) # Request [newer major].[newer minor].0.[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.0.7  0  0  0  0) # Request [newer major].[newer minor].0.[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.0.9  0  0  0  0) # Request [newer major].[newer minor].0.[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.2.0  0  0  0  0) # Request [newer major].[newer minor].[older patch].0
test_write_basic_config_version_file(4.05.06.007  9.9.2.2  0  0  0  0) # Request [newer major].[newer minor].[older patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.2.7  0  0  0  0) # Request [newer major].[newer minor].[older patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.2.9  0  0  0  0) # Request [newer major].[newer minor].[older patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.6.0  0  0  0  0) # Request [newer major].[newer minor].[same patch].0
test_write_basic_config_version_file(4.05.06.007  9.9.6.2  0  0  0  0) # Request [newer major].[newer minor].[same patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.6.7  0  0  0  0) # Request [newer major].[newer minor].[same patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.6.9  0  0  0  0) # Request [newer major].[newer minor].[same patch].[newer tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.9.0  0  0  0  0) # Request [newer major].[newer minor].[newer patch].0
test_write_basic_config_version_file(4.05.06.007  9.9.9.2  0  0  0  0) # Request [newer major].[newer minor].[newer patch].[older tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.9.7  0  0  0  0) # Request [newer major].[newer minor].[newer patch].[same tweak]
test_write_basic_config_version_file(4.05.06.007  9.9.9.9  0  0  0  0) # Request [newer major].[newer minor].[newer patch].[newer tweak]

test_write_basic_config_version_file(4        0...5       1  0  0  0)
test_write_basic_config_version_file(4        2...5       1  0  0  0)
test_write_basic_config_version_file(4        2...4       1  0  0  0)
test_write_basic_config_version_file(4        4...<5      1  1  0  0)
test_write_basic_config_version_file(4        9...10      0  0  0  0)

test_write_basic_config_version_file(4        0.1...5     1  0  0  0)
test_write_basic_config_version_file(4        2.1...5     1  0  0  0)
test_write_basic_config_version_file(4        2.8...5     1  0  0  0)
test_write_basic_config_version_file(4        2.1...4     1  0  0  0)
test_write_basic_config_version_file(4        2.8...4     1  0  0  0)
test_write_basic_config_version_file(4        4.0...<5    1  1  0  0)
test_write_basic_config_version_file(4        4.8...<5    0  0  0  0)
test_write_basic_config_version_file(4        4.1...5     0  0  0  0)
test_write_basic_config_version_file(4        4.8...5     0  0  0  0)
test_write_basic_config_version_file(4        9.1...10    0  0  0  0)
test_write_basic_config_version_file(4        9.8...10    0  0  0  0)


test_write_basic_config_version_file(4.05     0.1...5     1  0  0  0)
test_write_basic_config_version_file(4.05     2.1...5     1  0  0  0)
test_write_basic_config_version_file(4.05     2.8...5     1  0  0  0)
test_write_basic_config_version_file(4.05     2.1...4     0  0  0  0)
test_write_basic_config_version_file(4.05     2.8...4     0  0  0  0)
test_write_basic_config_version_file(4.05     2.8...4.8   1  0  0  0)
test_write_basic_config_version_file(4.05     4.1...<5    1  1  0  0)
test_write_basic_config_version_file(4.05     4.8...<5    0  0  0  0)
test_write_basic_config_version_file(4.05     4.5...4.5.8 1  1  1  0)
test_write_basic_config_version_file(4.05     4.5...<4.6  1  1  1  0)
test_write_basic_config_version_file(4.05     4.1...5     1  0  0  0)
test_write_basic_config_version_file(4.05     4.8...5     0  0  0  0)
test_write_basic_config_version_file(4.05     9.1...10    0  0  0  0)
test_write_basic_config_version_file(4.05     9.8...10    0  0  0  0)


test_write_basic_config_version_file(4.5.06    0.1...5     1  0  0  0)
test_write_basic_config_version_file(4.5.06    2.1...5     1  0  0  0)
test_write_basic_config_version_file(4.5.06    2.8...5     1  0  0  0)
test_write_basic_config_version_file(4.5.06    2.1...4     0  0  0  0)
test_write_basic_config_version_file(4.5.06    2.8...4     0  0  0  0)
test_write_basic_config_version_file(4.5.06    2.8...4.8   1  0  0  0)
test_write_basic_config_version_file(4.5.06    4.1...<5    1  1  0  0)
test_write_basic_config_version_file(4.5.06    4.8...<5    0  0  0  0)
test_write_basic_config_version_file(4.5.06    4.5...4.5.4 0  0  0  0)
test_write_basic_config_version_file(4.5.06    4.5...4.5.8 1  1  1  0)
test_write_basic_config_version_file(4.5.06    4.5...<4.6  1  1  1  0)
test_write_basic_config_version_file(4.5.06    4.1...5     1  0  0  0)
test_write_basic_config_version_file(4.5.06    4.8...5     0  0  0  0)
test_write_basic_config_version_file(4.5.06    9.1...10    0  0  0  0)
test_write_basic_config_version_file(4.5.06    9.8...10    0  0  0  0)
