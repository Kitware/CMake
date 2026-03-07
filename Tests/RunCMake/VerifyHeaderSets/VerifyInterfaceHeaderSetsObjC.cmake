cmake_policy(SET CMP0209 NEW)

enable_language(C CXX OBJC OBJCXX)

add_library(lang_test_objc STATIC lib.m)
target_sources(lang_test_objc INTERFACE FILE_SET HEADERS FILES lang_test.h)
target_compile_definitions(lang_test_objc INTERFACE EXPECT_OBJC)

# OBJC + OBJCXX sources -> lattice promotes to OBJCXX for unlanguaged headers
add_library(lang_test_objcxx STATIC lib.m lib.mm)
target_sources(lang_test_objcxx INTERFACE FILE_SET HEADERS FILES lang_test.h)
target_compile_definitions(lang_test_objcxx INTERFACE EXPECT_CXX EXPECT_OBJC)
