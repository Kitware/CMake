
# No known reference for AppleClang versions.
# Generic reference: http://clang.llvm.org/cxx_status.html
# http://clang.llvm.org/docs/LanguageExtensions.html

set(_cmake_oldestSupported "((__clang_major__ * 100) + __clang_minor__) >= 501")

include("${CMAKE_CURRENT_LIST_DIR}/Clang-CXX-TestableFeatures.cmake")

set(AppleClang51_CXX14 "((__clang_major__ * 100) + __clang_minor__) >= 501 && __cplusplus > 201103L")
# http://llvm.org/bugs/show_bug.cgi?id=19242
set(_cmake_feature_test_cxx_attribute_deprecated "${AppleClang51_CXX14}")
# http://llvm.org/bugs/show_bug.cgi?id=19698
set(_cmake_feature_test_cxx_decltype_auto "${AppleClang51_CXX14}")
set(_cmake_feature_test_cxx_digit_separators "${AppleClang51_CXX14}")
# http://llvm.org/bugs/show_bug.cgi?id=19674
set(_cmake_feature_test_cxx_generic_lambdas "${AppleClang51_CXX14}")

set(AppleClang51_CXX11 "${_cmake_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_enum_forward_declarations "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_sizeof_member "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_extended_friend_declarations "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_extern_templates "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_func_identifier "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_inline_namespaces "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_long_long_type "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_right_angle_brackets "${AppleClang51_CXX11}")
set(_cmake_feature_test_cxx_variadic_macros "${AppleClang51_CXX11}")

set(AppleClang_CXX98 "${_cmake_oldestSupported} && __cplusplus >= 199711L")
set(_cmake_feature_test_cxx_template_template_parameters "${AppleClang_CXX98}")
