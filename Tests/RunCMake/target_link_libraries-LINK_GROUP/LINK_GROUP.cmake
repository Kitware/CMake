enable_language(C)

# ensure command line is always displayed and do not use any response file
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

if (CMAKE_GENERATOR MATCHES "Borland|NMake")
  string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
  string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
endif()


add_library(base1 SHARED base.c)
add_library(base2 SHARED base.c)


set(CMAKE_C_LINK_GROUP_USING_feat1 "--START_GROUP" "--END_GROUP")
set(CMAKE_C_LINK_GROUP_USING_feat1_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat1 "--LIBFLAG<LIBRARY>")
set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat2 "--PREFIX_LIBRARY" "--LIBFLAG<LIBRARY>" "--SUFFIX_LIBRARY")
set(CMAKE_C_LINK_LIBRARY_USING_feat2_SUPPORTED TRUE)

set(CMAKE_C_LINK_GROUP_USING_feat2 "--START_GROUP" "--END_GROUP")
set(CMAKE_LINK_GROUP_USING_feat2 "--START_GROUP" "--END_GROUP")
set(CMAKE_LINK_GROUP_USING_feat2_SUPPORTED TRUE)

add_library(LinkGroup_simple1 SHARED lib.c)
target_link_libraries(LinkGroup_simple1 PRIVATE "$<LINK_GROUP:feat1,base1,base2>")


add_library(base3 SHARED base.c)
target_link_libraries(base3 PUBLIC base1)
add_library(LinkGroup_simple2 SHARED lib.c)
target_link_libraries(LinkGroup_simple2 PRIVATE "$<LINK_GROUP:feat1,base2,base3>")


add_library(LinkGroup_multiple-definitions SHARED lib.c)
target_link_libraries(LinkGroup_multiple-definitions PRIVATE "$<LINK_GROUP:feat2,base1,base2>")


add_library(base4 SHARED base.c)
target_link_libraries(base4 INTERFACE "$<LINK_GROUP:feat1,base1,base2>")
add_library(LinkGroup_multiple-groups SHARED lib.c)
target_link_libraries(LinkGroup_multiple-groups PRIVATE "$<LINK_GROUP:feat1,base2,base4>")


add_library(base5 SHARED base.c)
target_link_libraries(base5 PUBLIC base1)
add_library(LinkGroup_group-and-single SHARED lib.c)
target_link_libraries(LinkGroup_group-and-single PRIVATE "$<LINK_GROUP:feat1,base1,base3>" base5)


add_library(LinkGroup_with-LINK_LIBRARY SHARED lib.c)
target_link_libraries(LinkGroup_with-LINK_LIBRARY PRIVATE "$<LINK_GROUP:feat1,$<LINK_LIBRARY:feat1,base1>,base2>")

add_library(LinkGroup_with-LINK_LIBRARY2 SHARED lib.c)
target_link_libraries(LinkGroup_with-LINK_LIBRARY2 PRIVATE "$<LINK_GROUP:feat1,$<LINK_LIBRARY:feat2,base1,base2>>")


add_library(LinkGroup_with-LINK_LIBRARY_OVERRIDE SHARED lib.c)
target_link_libraries(LinkGroup_with-LINK_LIBRARY_OVERRIDE PRIVATE "$<LINK_GROUP:feat1,$<LINK_LIBRARY:feat1,base1,base3>,base2>")
set_property(TARGET LinkGroup_with-LINK_LIBRARY_OVERRIDE PROPERTY LINK_LIBRARY_OVERRIDE_base1 feat1)
