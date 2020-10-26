set(RESULT_FILES
  "${RunCMake_TEST_BINARY_DIR}/GoogleTestXMLSpecial/cases.case/0.xml"
  "${RunCMake_TEST_BINARY_DIR}/GoogleTestXMLSpecial/cases.case/1.xml"
  "${RunCMake_TEST_BINARY_DIR}/GoogleTestXMLSpecial/cases.case/2.xml"
)

# Check result files exist
foreach(file ${RESULT_FILES})
  if(NOT EXISTS ${file})
    if(NOT ${RunCMake_TEST_FAILED} STREQUAL "")
      set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}\n")
    endif()
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}Result XML file ${file} was not created")
  endif()
endforeach()

# and no other xml files are created
file(GLOB_RECURSE file_list "${RunCMake_TEST_BINARY_DIR}/GoogleTestXMLSpecial/*/*.xml" LIST_DIRECTORIES false)

foreach(file ${file_list})
  list(FIND RESULT_FILES "${file}" idx)
  if(-1 EQUAL ${idx})
    if(NOT ${RunCMake_TEST_FAILED} STREQUAL "")
      set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}\n")
    endif()
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}Invalid file ${file} was created")
  endif()
endforeach()
