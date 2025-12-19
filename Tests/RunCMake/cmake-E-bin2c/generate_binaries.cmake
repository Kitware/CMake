enable_language(C)
enable_language(CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/hashes.cmake")

function(verify_long_c filename expected_lf_hash expected_crlf_hash)
  file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/${filename}" actual_hash)
  if(NOT actual_hash STREQUAL expected_lf_hash AND NOT actual_hash STREQUAL expected_crlf_hash)
    message(FATAL_ERROR "File ${CMAKE_CURRENT_SOURCE_DIR}/${filename} does not match expected hash and has likely been manually edited. Edit and re-run ${CMAKE_CURRENT_SOURCE_DIR}/generate_files.sh instead.")
  endif()
endfunction()

verify_long_c(long.c.txt "${long_c_lf_hash}" "${long_c_crlf_hash}")
verify_long_c(long_signed.c.txt "${long_signed_c_lf_hash}" "${long_signed_c_crlf_hash}")
verify_long_c(long_decimal.c.txt "${long_decimal_c_lf_hash}" "${long_decimal_c_crlf_hash}")
verify_long_c(long_signed_decimal.c.txt "${long_signed_decimal_c_lf_hash}" "${long_signed_decimal_c_crlf_hash}")

add_executable(generate_binary generate_binary.cpp)

function(generate_binary name)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.bin"
    COMMAND generate_binary "${name}" "${CMAKE_CURRENT_BINARY_DIR}/${name}.bin"
    DEPENDS generate_binary
    )
  add_custom_target(
    verify_${name} ALL
    COMMAND "${CMAKE_COMMAND}"
      "-DFILENAME=${CMAKE_CURRENT_BINARY_DIR}/${name}.bin"
      "-DSHA256SUM=${${name}_hash}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_binary.cmake"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.bin"
    )
endfunction()

foreach(name IN ITEMS basic empty text_lf text_crlf text_align long)
  generate_binary("${name}")
endforeach()

add_executable(verify_long_variants verify_long_variants.c)
add_custom_target(verify_long_variants_contents ALL COMMAND verify_long_variants)

if(CMake_TEST_BIN2C_LARGE_FILE)
  include("${CMAKE_CURRENT_SOURCE_DIR}/very_long_params.cmake")

  add_executable(generate_very_long generate_very_long.cpp)
  target_compile_definitions(generate_very_long PRIVATE JACK_COUNT=${jack_count})
  add_custom_target(
    verify_very_long_hash
    COMMAND "${CMAKE_COMMAND}"
      "-DGENERATE_VERY_LONG=$<TARGET_FILE:generate_very_long>"
      "-DSHA256SUM=${very_long_hash}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_very_long_hash.cmake"
    )

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/very_long_executables.cmake"
    COMMAND "${CMAKE_COMMAND}"
      "-DGENERATE_VERY_LONG=$<TARGET_FILE:generate_very_long>"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/record_very_long.cmake"
    )
  add_custom_target(
    record_very_long ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/very_long_executables.cmake"
    )
endif()
