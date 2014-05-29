
# Reference: http://clang.llvm.org/cxx_status.html
# http://clang.llvm.org/docs/LanguageExtensions.html

set(testable_features
  cxx_alias_templates
  cxx_alignas
  cxx_attributes
  cxx_auto_type
  cxx_binary_literals
  cxx_constexpr
  cxx_contextual_conversions
  cxx_decltype
  cxx_decltype_incomplete_return_types
  cxx_default_function_template_args
  cxx_defaulted_functions
  cxx_delegating_constructors
  cxx_deleted_functions
  cxx_explicit_conversions
  cxx_generalized_initializers
  cxx_inheriting_constructors
  cxx_lambdas
  cxx_local_type_template_args
  cxx_noexcept
  cxx_nonstatic_member_init
  cxx_nullptr
  cxx_range_for
  cxx_raw_string_literals
  cxx_reference_qualified_functions
  cxx_relaxed_constexpr
  cxx_return_type_deduction
  cxx_rvalue_references
  cxx_static_assert
  cxx_strong_enums
  cxx_thread_local
  cxx_unicode_literals
  cxx_unrestricted_unions
  cxx_user_literals
  cxx_variable_templates
  cxx_variadic_templates
)

set(_cmake_oldestSupported "((__clang_major__ * 100) + __clang_minor__) >= 304")

foreach(feature ${testable_features})
  set(_cmake_feature_test_${feature} "${_cmake_oldestSupported} && __has_feature(${feature})")
endforeach()

unset(testable_features)

set(_cmake_feature_test_cxx_aggregate_default_initializers "${_cmake_oldestSupported} && __has_feature(cxx_aggregate_nsdmi)")

set(_cmake_feature_test_cxx_trailing_return_types "${_cmake_oldestSupported} && __has_feature(cxx_trailing_return)")
set(_cmake_feature_test_cxx_alignof "${_cmake_oldestSupported} && __has_feature(cxx_alignas)")
set(_cmake_feature_test_cxx_final "${_cmake_oldestSupported} && __has_feature(cxx_override_control)")
set(_cmake_feature_test_cxx_override "${_cmake_oldestSupported} && __has_feature(cxx_override_control)")
set(_cmake_feature_test_cxx_uniform_initialization "${_cmake_oldestSupported} && __has_feature(cxx_generalized_initializers)")
set(_cmake_feature_test_cxx_defaulted_move_initializers "${_cmake_oldestSupported} && __has_feature(cxx_defaulted_functions)")
set(_cmake_feature_test_cxx_lambda_init_captures "${_cmake_oldestSupported} && __has_feature(cxx_init_captures)")

set(Clang34_CXX14 "((__clang_major__ * 100) + __clang_minor__) >= 304 && __cplusplus > 201103L")
# http://llvm.org/bugs/show_bug.cgi?id=19242
set(_cmake_feature_test_cxx_attribute_deprecated "${Clang34_CXX14}")
# http://llvm.org/bugs/show_bug.cgi?id=19698
set(_cmake_feature_test_cxx_decltype_auto "${Clang34_CXX14}")
set(_cmake_feature_test_cxx_digit_separators "${Clang34_CXX14}")
# http://llvm.org/bugs/show_bug.cgi?id=19674
set(_cmake_feature_test_cxx_generic_lambdas "${Clang34_CXX14}")

# TODO: Should be supported by Clang 3.1
set(Clang31_CXX11 "${_cmake_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_enum_forward_declarations "${Clang31_CXX11}")
set(_cmake_feature_test_cxx_sizeof_member "${Clang31_CXX11}")
# TODO: Should be supported by Clang 2.9
set(Clang29_CXX11 "${_cmake_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_extended_friend_declarations "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_extern_templates "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_func_identifier "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_inline_namespaces "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_long_long_type "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_right_angle_brackets "${Clang29_CXX11}")
set(_cmake_feature_test_cxx_variadic_macros "${Clang29_CXX11}")

# TODO: Should be supported forever?
set(Clang_CXX98 "${_cmake_oldestSupported} && __cplusplus >= 199711L")
set(_cmake_feature_test_cxx_template_template_parameters "${Clang_CXX98}")
