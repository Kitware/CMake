set(EXPECTED_FILES_COUNT_MONOLITHIC "1")
set(EXPECTED_FILES_COUNT_COMPONENT "2")
set(EXPECTED_FILES_COUNT "${EXPECTED_FILES_COUNT_${PACKAGING_TYPE}}")

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  set(EXPECTED_FILE_1 "deb_description-0.1.1-*-satu.deb")
  set(EXPECTED_FILE_2 "deb_description-0.1.1-*-dua.deb")
  set(EXPECTED_FILE_CONTENT_1_LIST "/satu;/satu/CMakeLists.txt")
  set(EXPECTED_FILE_CONTENT_2_LIST "/dua;/dua/CMakeLists.txt")

elseif(PACKAGING_TYPE STREQUAL "MONOLITHIC")
  set(EXPECTED_FILE_CONTENT_1_LIST "/dua;/dua/CMakeLists.txt;/satu;/satu/CMakeLists.txt")

endif()

# kate: indent-width 2;
