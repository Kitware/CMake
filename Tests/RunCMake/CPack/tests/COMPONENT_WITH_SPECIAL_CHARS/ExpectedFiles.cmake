if(PACKAGING_TYPE STREQUAL "MONOLITHIC")
  set(EXPECTED_FILES_COUNT "1")
  set(EXPECTED_FILE_CONTENT_1_LIST
    "/foo1"
    "/foo1/CMakeLists.txt"
    "/foo2"
    "/foo2/CMakeLists.txt"
    "/foo3"
    "/foo3/CMakeLists.txt"
  )
elseif(PACKAGING_TYPE STREQUAL "COMPONENT")
  set(EXPECTED_FILES_COUNT "3")
  set(EXPECTED_FILE_1 "*-comp1.test1.*")
  set(EXPECTED_FILE_CONTENT_1_LIST
    "/foo1"
    "/foo1/CMakeLists.txt"
  )
  set(EXPECTED_FILE_2 "*-component2.*")
  set(EXPECTED_FILE_CONTENT_2_LIST
    "/foo2"
    "/foo2/CMakeLists.txt"
  )
  set(EXPECTED_FILE_3 "*-component3.*")
  set(EXPECTED_FILE_CONTENT_3_LIST
    "/foo3"
    "/foo3/CMakeLists.txt"
  )
elseif(PACKAGING_TYPE STREQUAL "GROUP")
  set(EXPECTED_FILES_COUNT "2")
  set(EXPECTED_FILE_1 "*-group1.*")
  set(EXPECTED_FILE_CONTENT_1_LIST
    "/foo1"
    "/foo1/CMakeLists.txt"
    "/foo2"
    "/foo2/CMakeLists.txt"
  )
  set(EXPECTED_FILE_2 "*-group2.*")
  set(EXPECTED_FILE_CONTENT_2_LIST
    "/foo3"
    "/foo3/CMakeLists.txt"
  )
endif()
