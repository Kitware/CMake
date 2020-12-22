file(READ ${generatedFile} actualContent HEX)
if(NOT "${actualContent}" STREQUAL "${expectedContent}")
    message(SEND_ERROR "Content mismatch actual: \"${actualContent}\" expected: \"${expectedContent}\"")
endif()
