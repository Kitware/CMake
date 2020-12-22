function(generate_from_file in out)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/file_ip.txt "${in}")
  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/file_op.txt
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/file_ip.txt
    NEWLINE_STYLE WIN32
    )

  add_custom_target(verifyContentFromFile ALL
    COMMAND ${CMAKE_COMMAND}
      -DgeneratedFile=${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/file_op.txt
      -DexpectedContent=${out}
      -P "${CMAKE_CURRENT_SOURCE_DIR}/VerifyContent.cmake"
    )
endfunction()

function(generate_from_content in out)
  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/content_op.txt
    CONTENT ${in}
    NEWLINE_STYLE WIN32
    )

  add_custom_target(verifyContentFromContent ALL
    COMMAND ${CMAKE_COMMAND}
      -DgeneratedFile=${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/content_op.txt
      -DexpectedContent=${out}
      -P "${CMAKE_CURRENT_SOURCE_DIR}/VerifyContent.cmake"
    )
endfunction()

generate_from_file("a" "610d0a") # 62->b, 0d0a->\r\n
generate_from_content("a" "610d0a")
