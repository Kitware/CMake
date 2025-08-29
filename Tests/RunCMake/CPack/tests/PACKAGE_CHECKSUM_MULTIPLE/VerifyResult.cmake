set(hash_algos MD5 SHA1 SHA224 SHA256 SHA384 SHA512)

file(GLOB PACKAGE RELATIVE "${bin_dir}" "*.tar.gz")

foreach(algo IN LISTS hash_algos)
  string(TOLOWER ${algo} CHECKSUM_EXTENSION)
  file(STRINGS ${PACKAGE}.${CHECKSUM_EXTENSION} CHSUM_VALUE)
  file(${algo} ${PACKAGE} expected_value)
  set(expected_value "${expected_value}  ${PACKAGE}")

  if(NOT expected_value STREQUAL CHSUM_VALUE)
    message(FATAL_ERROR "Generated checksum is not valid! Expected [${expected_value}] Got [${CHSUM_VALUE}]")
  endif()
endforeach()
