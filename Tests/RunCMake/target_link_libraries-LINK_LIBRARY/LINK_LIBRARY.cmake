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

set(CMAKE_C_LINK_LIBRARY_USING_feat1 "--LIBFLAG<LIBRARY>")
set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat1_1 "--LIBFLAG_C<LIBRARY>")
set(CMAKE_LINK_LIBRARY_USING_feat1_1 "--LIBFLAG<LIBRARY>")
set(CMAKE_LINK_LIBRARY_USING_feat1_1_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat2 "--PREFIXGROUP" "--LIBGROUP<LIBRARY>" "--SUFFIXGROUP")
set(CMAKE_C_LINK_LIBRARY_USING_feat2_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat3 "--PREFIXGROUP" "<LINK_ITEM>" "--SUFFIXGROUP")
set(CMAKE_C_LINK_LIBRARY_USING_feat3_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat4 "--PREFIXGROUP" "--LIBFLAG<LIBRARY> --ITEMFLAG<LIB_ITEM>" "--SUFFIXGROUP")
set(CMAKE_C_LINK_LIBRARY_USING_feat4_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat5 "--PREFIXGROUP" "PATH{--LIBFLAG<LIBRARY>}NAME{--ITEMFLAG<LIB_ITEM>}" "--SUFFIXGROUP")
set(CMAKE_C_LINK_LIBRARY_USING_feat5_SUPPORTED TRUE)

set(CMAKE_C_LINK_LIBRARY_USING_feat6 "<LINK_ITEM>")
set(CMAKE_C_LINK_LIBRARY_USING_feat6_SUPPORTED TRUE)


add_library(LinkLibrary_simple1 SHARED lib.c)
target_link_libraries(LinkLibrary_simple1 PRIVATE "$<LINK_LIBRARY:feat1,base1>")

add_library(LinkLibrary_simple2 SHARED lib.c)
target_link_libraries(LinkLibrary_simple2 PRIVATE "$<LINK_LIBRARY:feat1_1,base1>")

add_library(LinkLibrary_group1 SHARED lib.c)
target_link_libraries(LinkLibrary_group1 PRIVATE "$<LINK_LIBRARY:feat1,base1,base2>")

add_library(LinkLibrary_group2 SHARED lib.c)
target_link_libraries(LinkLibrary_group2 PRIVATE "$<LINK_LIBRARY:feat2,base1,base2>")

add_library(LinkLibrary_nested_feature1 SHARED lib.c)
target_link_libraries(LinkLibrary_nested_feature1 PRIVATE "$<LINK_LIBRARY:feat1,base1,$<LINK_LIBRARY:feat1,base2>>")

add_library(LinkLibrary_nested_feature2 SHARED lib.c)
target_link_libraries(LinkLibrary_nested_feature2 PRIVATE "$<LINK_LIBRARY:feat2,base1,$<LINK_LIBRARY:feat2,base2>>")

add_library(LinkLibrary_link_items1 SHARED lib.c)
target_link_libraries(LinkLibrary_link_items1 PRIVATE "$<LINK_LIBRARY:feat3,base1,other>")

add_library(LinkLibrary_link_items2 SHARED lib.c)
target_link_libraries(LinkLibrary_link_items2 PRIVATE "$<LINK_LIBRARY:feat2,base1,other>")

add_library(LinkLibrary_link_items3 SHARED lib.c)
target_link_libraries(LinkLibrary_link_items3 PRIVATE "$<LINK_LIBRARY:feat4,base1,other>")

add_library(LinkLibrary_link_items4 SHARED lib.c)
target_link_libraries(LinkLibrary_link_items4 PRIVATE "$<LINK_LIBRARY:feat5,base1,other>")

add_library(base3 SHARED base.c)
target_link_libraries(base3 PRIVATE "$<LINK_LIBRARY:feat6,base1>")
add_library(LinkLibrary_mix_features1 SHARED lib.c)
target_link_libraries(LinkLibrary_mix_features1 PRIVATE "$<LINK_LIBRARY:feat2,base1,base3>")

target_link_libraries(base3 INTERFACE "$<LINK_LIBRARY:feat2,base1>")
add_library(LinkLibrary_mix_features2 SHARED lib.c)
target_link_libraries(LinkLibrary_mix_features2 PRIVATE "$<LINK_LIBRARY:feat2,base1,base3>")

target_link_libraries(base3 INTERFACE other1)
add_library(LinkLibrary_mix_features3 SHARED lib.c)
target_link_libraries(LinkLibrary_mix_features3 PRIVATE base2 "$<LINK_LIBRARY:feat2,base1,base3>" other2)

# testing LINK_LIBRARY_OVERRIDE property
add_library(LinkLibrary_override_features1 SHARED lib.c)
target_link_libraries(LinkLibrary_override_features1 PRIVATE "$<LINK_LIBRARY:feat1,base1,base3>")
set_property(TARGET LinkLibrary_override_features1 PROPERTY LINK_LIBRARY_OVERRIDE "feat1,base1")

add_library(LinkLibrary_override_features2 SHARED lib.c)
target_link_libraries(LinkLibrary_override_features2 PRIVATE "$<LINK_LIBRARY:feat1,base1,base3>")
set_property(TARGET LinkLibrary_override_features2 PROPERTY LINK_LIBRARY_OVERRIDE "feat2,base1,other1")

add_library(LinkLibrary_override_with_default SHARED lib.c)
target_link_libraries(LinkLibrary_override_with_default PRIVATE "$<LINK_LIBRARY:feat1,base1,base3>")
set_property(TARGET LinkLibrary_override_with_default PROPERTY LINK_LIBRARY_OVERRIDE "$<$<LINK_LANGUAGE:C>:DEFAULT,base1,other1>")

# testing LINK_LIBRARY_OVERRIDE_<LIBRARY> property
add_library(LinkLibrary_override_features3 SHARED lib.c)
target_link_libraries(LinkLibrary_override_features3 PRIVATE "$<LINK_LIBRARY:feat1,base1,base3>")
set_property(TARGET LinkLibrary_override_features3 PROPERTY LINK_LIBRARY_OVERRIDE_base1 feat1)

add_library(LinkLibrary_override_features4 SHARED lib.c)
target_link_libraries(LinkLibrary_override_features4 PRIVATE "$<LINK_LIBRARY:feat1,base1,base3>")
set_property(TARGET LinkLibrary_override_features4 PROPERTY LINK_LIBRARY_OVERRIDE "feat3,base1,other1")
set_property(TARGET LinkLibrary_override_features4 PROPERTY LINK_LIBRARY_OVERRIDE_base1 feat2)
set_property(TARGET LinkLibrary_override_features4 PROPERTY LINK_LIBRARY_OVERRIDE_other1 feat2)
