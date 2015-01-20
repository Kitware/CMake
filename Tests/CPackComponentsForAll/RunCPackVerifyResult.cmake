message(STATUS "=============================================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")

if(NOT CPackComponentsForAll_BINARY_DIR)
  message(FATAL_ERROR "CPackComponentsForAll_BINARY_DIR not set")
endif()

if(NOT CPackGen)
  message(FATAL_ERROR "CPackGen not set")
endif()

message("CMAKE_CPACK_COMMAND = ${CMAKE_CPACK_COMMAND}")
if(NOT CMAKE_CPACK_COMMAND)
  message(FATAL_ERROR "CMAKE_CPACK_COMMAND not set")
endif()

if(NOT CPackComponentWay)
  message(FATAL_ERROR "CPackComponentWay not set")
endif()

set(expected_file_mask "")
# The usual default behavior is to expect a single file
# Then some specific generators (Archive, RPM, ...)
# May produce several numbers of files depending on
# CPACK_COMPONENT_xxx values
set(expected_count 1)
set(config_type $ENV{CMAKE_CONFIG_TYPE})
set(config_args )
if(config_type)
  set(config_args -C ${config_type})
endif()
set(config_verbose )

if(CPackGen MATCHES "ZIP")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.zip")
    if (${CPackComponentWay} STREQUAL "default")
        set(expected_count 1)
    elseif (${CPackComponentWay} STREQUAL "OnePackPerGroup")
        set(expected_count 3)
    elseif (${CPackComponentWay} STREQUAL "IgnoreGroup")
        set(expected_count 4)
    elseif (${CPackComponentWay} STREQUAL "AllInOne")
        set(expected_count 1)
    endif ()
elseif (CPackGen MATCHES "RPM")
    set(config_verbose -D "CPACK_RPM_PACKAGE_DEBUG=1")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.rpm")
    if (${CPackComponentWay} STREQUAL "default")
        set(expected_count 1)
    elseif (${CPackComponentWay} STREQUAL "OnePackPerGroup")
        set(expected_count 3)
    elseif (${CPackComponentWay} STREQUAL "IgnoreGroup")
        set(expected_count 4)
    elseif (${CPackComponentWay} STREQUAL "AllInOne")
        set(expected_count 1)
    endif ()
elseif (CPackGen MATCHES "DEB")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.deb")
    if (${CPackComponentWay} STREQUAL "default")
        set(expected_count 1)
    elseif (${CPackComponentWay} STREQUAL "OnePackPerGroup")
        set(expected_count 3)
    elseif (${CPackComponentWay} STREQUAL "IgnoreGroup")
        set(expected_count 4)
    elseif (${CPackComponentWay} STREQUAL "AllInOne")
        set(expected_count 1)
    endif ()
endif()

if(CPackGen MATCHES "DragNDrop")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.dmg")
    if (${CPackComponentWay} STREQUAL "default")
        set(expected_count 1)
    elseif (${CPackComponentWay} STREQUAL "OnePackPerGroup")
        set(expected_count 3)
    elseif (${CPackComponentWay} STREQUAL "IgnoreGroup")
        set(expected_count 4)
    elseif (${CPackComponentWay} STREQUAL "AllInOne")
        set(expected_count 1)
    endif ()
endif()

# clean-up previously CPack generated files
if(expected_file_mask)
  file(GLOB expected_file "${expected_file_mask}")
  if (expected_file)
    file(REMOVE ${expected_file})
  endif()
endif()

message("config_args = ${config_args}")
message("config_verbose = ${config_verbose}")
execute_process(COMMAND ${CMAKE_CPACK_COMMAND} ${config_verbose} -G ${CPackGen} ${config_args}
    RESULT_VARIABLE CPack_result
    OUTPUT_VARIABLE CPack_output
    ERROR_VARIABLE CPack_error
    WORKING_DIRECTORY ${CPackComponentsForAll_BINARY_DIR})

if (CPack_result)
  message(FATAL_ERROR "error: CPack execution went wrong!, CPack_output=${CPack_output}, CPack_error=${CPack_error}")
else ()
  message(STATUS "CPack_output=${CPack_output}")
endif()

# Now verify if the number of expected file is OK
# - using expected_file_mask and
# - expected_count
if(expected_file_mask)
  file(GLOB expected_file "${expected_file_mask}")

  message(STATUS "expected_count='${expected_count}'")
  message(STATUS "expected_file='${expected_file}'")
  message(STATUS "expected_file_mask='${expected_file_mask}'")

  if(NOT expected_file)
    message(FATAL_ERROR "error: expected_file=${expected_file} does not exist: CPackComponentsForAll test fails. (CPack_output=${CPack_output}, CPack_error=${CPack_error}")
  endif()

  list(LENGTH expected_file actual_count)
  message(STATUS "actual_count='${actual_count}'")
  if(NOT actual_count EQUAL expected_count)
    message(FATAL_ERROR "error: expected_count=${expected_count} does not match actual_count=${actual_count}: CPackComponents test fails. (CPack_output=${CPack_output}, CPack_error=${CPack_error})")
  endif()
endif()

# Validate content
if(CPackGen MATCHES "RPM")
  find_program(RPM_EXECUTABLE rpm)
  if(NOT RPM_EXECUTABLE)
    message(FATAL_ERROR "error: missing rpm executable required by the test")
  endif()

  set(CPACK_RPM_PACKAGE_SUMMARY "default summary")
  set(CPACK_RPM_libraries_PACKAGE_SUMMARY "libraries summary")
  set(CPACK_RPM_libraries_PACKAGE_DESCRIPTION "libraries description")
  set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION
    "An extremely useful application that makes use of MyLib")
  set(CPACK_COMPONENT_HEADERS_DESCRIPTION
    "C/C\\+\\+ header files for use with MyLib")

  if(${CPackComponentWay} STREQUAL "IgnoreGroup")
    # set gnu install prefixes to what they are set during rpm creation
    # CMAKE_SIZEOF_VOID_P is not set here but lib is prefix of lib64 so
    # relocation path test won't fail on OSes with lib64 library location
    include(GNUInstallDirs)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    foreach(check_file ${expected_file})
      string(REGEX MATCH ".*libraries.*" check_file_libraries_match ${check_file})
      string(REGEX MATCH ".*headers.*" check_file_headers_match ${check_file})
      string(REGEX MATCH ".*applications.*" check_file_applications_match ${check_file})
      string(REGEX MATCH ".*Unspecified.*" check_file_Unspecified_match ${check_file})

      execute_process(COMMAND ${RPM_EXECUTABLE} -pqi ${check_file}
          OUTPUT_VARIABLE check_file_content
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

      if(check_file_libraries_match)
        set(check_file_match_expected_summary ".*${CPACK_RPM_libraries_PACKAGE_SUMMARY}.*")
        set(check_file_match_expected_description ".*${CPACK_RPM_libraries_PACKAGE_DESCRIPTION}.*")
        set(check_file_match_expected_relocation_path "Relocations : ${CPACK_PACKAGING_INSTALL_PREFIX} ${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
        set(spec_regex "*libraries*")
      elseif(check_file_headers_match)
        set(check_file_match_expected_summary ".*${CPACK_RPM_PACKAGE_SUMMARY}.*")
        set(check_file_match_expected_description ".*${CPACK_COMPONENT_HEADERS_DESCRIPTION}.*")
        set(check_file_match_expected_relocation_path "Relocations : ${CPACK_PACKAGING_INSTALL_PREFIX} ${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
        set(spec_regex "*headers*")
      elseif(check_file_applications_match)
        set(check_file_match_expected_summary ".*${CPACK_RPM_PACKAGE_SUMMARY}.*")
        set(check_file_match_expected_description ".*${CPACK_COMPONENT_APPLICATIONS_DESCRIPTION}.*")
        set(check_file_match_expected_relocation_path "Relocations : ${CPACK_PACKAGING_INSTALL_PREFIX} ${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
        set(spec_regex "*applications*")
      elseif(check_file_Unspecified_match)
        set(check_file_match_expected_summary ".*${CPACK_RPM_PACKAGE_SUMMARY}.*")
        set(check_file_match_expected_description ".*DESCRIPTION.*")
        set(check_file_match_expected_relocation_path "Relocations : ${CPACK_PACKAGING_INSTALL_PREFIX} ${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
        set(spec_regex "*Unspecified*")
      else()
        message(FATAL_ERROR "error: unexpected rpm package '${check_file}'")
      endif()

      string(REGEX MATCH ${check_file_match_expected_summary} check_file_match_summary ${check_file_content})

      if(NOT check_file_match_summary)
        message(FATAL_ERROR "error: '${check_file}' rpm package summary does not match expected value - regex '${check_file_match_expected_summary}'; RPM output: '${check_file_content}'")
      endif()

      string(REGEX MATCH ${check_file_match_expected_description} check_file_match_description ${check_file_content})

      if(NOT check_file_match_description)
        message(FATAL_ERROR "error: '${check_file}' rpm package description does not match expected value - regex '${check_file_match_expected_description}'; RPM output: '${check_file_content}'")
      endif()

      string(REGEX MATCH ${check_file_match_expected_relocation_path} check_file_match_relocation_path ${check_file_content})

      if(NOT check_file_match_relocation_path)
        file(GLOB_RECURSE spec_file "${CPackComponentsForAll_BINARY_DIR}/${spec_regex}.spec")

        if(spec_file)
          file(READ ${spec_file} spec_file_content)
        endif()

        message(FATAL_ERROR "error: '${check_file}' rpm package relocation path does not match expected value - regex '${check_file_match_expected_relocation_path}'; RPM output: '${check_file_content}'; generated spec file: '${spec_file_content}'")
      endif()
    endforeach()
  elseif(${CPackComponentWay} STREQUAL "IgnoreGroup")
  endif()
endif()
