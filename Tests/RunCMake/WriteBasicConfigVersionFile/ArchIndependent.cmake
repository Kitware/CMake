# Hard-code architecture for test without a real compiler.
set(CMAKE_SIZEOF_VOID_P 4)

include(WriteBasicConfigVersionFile)

set(_dummy_version 1.0.0)

set(_compatibilities AnyNewerVersion
                     SameMajorVersion
                     SameMinorVersion
                     ExactVersion)

function(test_write_basic_config_version_file_arch_prepare filename_out compat arch_independent arch)
  if(arch_independent)
    set(arch_arg ARCH_INDEPENDENT)
  else()
    set(arch_arg )
  endif()

  set(filename "${CMAKE_CURRENT_BINARY_DIR}/${compat}Arch${arch_arg}ConfigVersion.cmake")

  set(CMAKE_SIZEOF_VOID_P "${arch}")

  write_basic_config_version_file("${filename}"
                                  VERSION "${_dummy_version}"
                                  COMPATIBILITY "${compat}"
                                  ${arch_arg})

  set("${filename_out}" "${filename}" PARENT_SCOPE)
endfunction()

function(test_write_basic_config_version_file_arch_check unsuitable_out filename arch)
  set(CMAKE_SIZEOF_VOID_P "${arch}")
  set(PACKAGE_FIND_VERSION "${_dummy_version}")

  include("${filename}")

  set("${unsuitable_out}" "${PACKAGE_VERSION_UNSUITABLE}" PARENT_SCOPE)
endfunction()

function(test_write_basic_config_version_file_arch_test expected_unsuitable compat arch_independent source_arch user_arch)
  test_write_basic_config_version_file_arch_prepare(filename "${compat}" "${arch_independent}" "${source_arch}")
  test_write_basic_config_version_file_arch_check(unsuitable "${filename}" "${user_arch}")
  if(unsuitable AND NOT expected_unsuitable)
    message(SEND_ERROR "Architecture was checked when it shouldn't have been. Compatibility: ${compat} ARCH_INDEPENDENT: ${arch_independent}.")
  elseif(expected_unsuitable AND NOT unsuitable)
    message(SEND_ERROR "Requested architecture check not performed. Compatibility: ${compat} ARCH_INDEPENDENT: ${arch_independent}.")
  endif()
endfunction()

set(_unsuitable TRUE)
set(_suitable FALSE)

foreach(compat ${_compatibilities})
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" TRUE 4 4)
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" FALSE 4 4)
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" TRUE 4 8)
  test_write_basic_config_version_file_arch_test("${_unsuitable}" "${compat}" FALSE 4 8)
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" TRUE 8 4)
  test_write_basic_config_version_file_arch_test("${_unsuitable}" "${compat}" FALSE 8 4)
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" TRUE 8 8)
  test_write_basic_config_version_file_arch_test("${_suitable}" "${compat}" FALSE 8 8)
endforeach()
