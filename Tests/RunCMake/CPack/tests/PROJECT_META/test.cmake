project(
  MetaInfoTest
  VERSION 1.2.3
  DESCRIPTION "This is going to be a summary"
  HOMEPAGE_URL "https://meta.test.info"
)
install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  set(CPACK_COMPONENTS_ALL test)
endif()
