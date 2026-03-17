function(expect CATEGORY EXPECTED)
  cmake_diagnostic(GET ${CATEGORY} actual)
  if(NOT "${actual}" STREQUAL "${EXPECTED}")
    message(SEND_ERROR
      "wrong action for diagnostic ${CATEGORY}"
      " (expected '${EXPECTED}', actual '${actual}')")
  endif()
endfunction()

function(expect_cached CATEGORY EXPECTED)
  expect(${CATEGORY} ${EXPECTED})

  if(NOT "${CATEGORY}=${EXPECTED}" IN_LIST CMAKE_DIAGNOSTIC_INIT)
    list(JOIN CMAKE_DIAGNOSTIC_INIT ", " pretty_state)
    message(SEND_ERROR
      "Cached state for ${CATEGORY} missing or incorrect"
      " (expected '${EXPECTED}' in ${pretty_state})")
  endif()
endfunction()
