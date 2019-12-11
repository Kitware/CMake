function(checkPackageURL FILE TAG EXPECTED_URL)
  getPackageInfo("${FILE}" "_file_info")
  string(REPLACE "\n" ";" _file_info "${_file_info}")

  set(_seen_url FALSE)
  foreach(_line IN LISTS _file_info)
    if(_line MATCHES "${TAG}: (.*)")
      set(_seen_url TRUE)
      if(NOT CMAKE_MATCH_1 STREQUAL EXPECTED_URL)
        message(FATAL_ERROR "Unexpected `Homepage` URL: `${CMAKE_MATCH_1}` != `${EXPECTED_URL}`")
      endif()
      break()
    endif()
  endforeach()
  if(NOT _seen_url)
    message(FATAL_ERROR "The packge `${FILE}` do not have URL as expected")
  endif()
endfunction()

if(GENERATOR_TYPE STREQUAL DEB)
  set(_tag " Homepage") # NOTE The leading space
elseif(GENERATOR_TYPE STREQUAL RPM)
  set(_tag "URL.*")
else()
  message(FATAL_ERROR "Unexpected CPack generator")
endif()

checkPackageURL("${FOUND_FILE_1}" "${_tag}" "https://meta.test.info")

# kate: indent-width 2;
