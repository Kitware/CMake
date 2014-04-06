
# Reference: http://msdn.microsoft.com/en-us/library/vstudio/hh567368.aspx
# http://blogs.msdn.com/b/vcblog/archive/2013/06/28/c-11-14-stl-features-fixes-and-breaking-changes-in-vs-2013.aspx

set(_oldestSupported "_MSC_VER >= 1600")

set(MSVC_2013 "_MSC_VER >= 1800")
set(_cmake_feature_test_cxx_alias_templates "${MSVC_2013}")
set(_cmake_feature_test_cxx_default_function_template_args "${MSVC_2013}")
set(_cmake_feature_test_cxx_defaulted_functions "${MSVC_2013}")
set(_cmake_feature_test_cxx_delegating_constructors "${MSVC_2013}")
set(_cmake_feature_test_cxx_deleted_functions "${MSVC_2013}")
set(_cmake_feature_test_cxx_explicit_conversions "${MSVC_2013}")
# http://thread.gmane.org/gmane.comp.lib.boost.devel/245202/focus=245221
set(_cmake_feature_test_cxx_generalized_initializers "${MSVC_2013}")
set(_cmake_feature_test_cxx_nonstatic_member_init "${MSVC_2013}")
set(_cmake_feature_test_cxx_raw_string_literals "${MSVC_2013}")
set(_cmake_feature_test_cxx_uniform_initialization "${MSVC_2013}")
# Possibly broken:
# http://thread.gmane.org/gmane.comp.lib.boost.devel/244986/focus=245333
set(_cmake_feature_test_cxx_variadic_templates "${MSVC_2013}")

set(MSVC_2012 "_MSC_VER >= 1700")
set(_cmake_feature_test_cxx_decltype_incomplete_return_types "${MSVC_2012}")
set(_cmake_feature_test_cxx_enum_forward_declarations "${MSVC_2012}")
set(_cmake_feature_test_cxx_final "${MSVC_2012}")
set(_cmake_feature_test_cxx_range_for "${MSVC_2012}")
set(_cmake_feature_test_cxx_strong_enums "${MSVC_2012}")

set(MSVC_2010 "_MSC_VER >= 1600")
set(_cmake_feature_test_cxx_auto_function "${MSVC_2010}")
set(_cmake_feature_test_cxx_auto_type "${MSVC_2010}")
set(_cmake_feature_test_cxx_decltype "${MSVC_2010}")
set(_cmake_feature_test_cxx_extended_friend_declarations "${MSVC_2010}")
set(_cmake_feature_test_cxx_extern_templates "${MSVC_2010}")
set(_cmake_feature_test_cxx_lambdas "${MSVC_2010}")
set(_cmake_feature_test_cxx_local_type_template_args "${MSVC_2010}")
set(_cmake_feature_test_cxx_long_long_type "${MSVC_2010}")
set(_cmake_feature_test_cxx_nullptr "${MSVC_2010}")
set(_cmake_feature_test_cxx_override "${MSVC_2010}")
set(_cmake_feature_test_cxx_right_angle_brackets "${MSVC_2010}")
set(_cmake_feature_test_cxx_rvalue_references "${MSVC_2010}")
set(_cmake_feature_test_cxx_static_assert "${MSVC_2010}")
set(_cmake_feature_test_cxx_template_template_parameters "${MSVC_2010}")
set(_cmake_feature_test_cxx_variadic_macros "${MSVC_2010}")

# Currently unsupported:
# http://herbsutter.com/2013/11/18/visual-c-compiler-november-2013-ctp/
# set(_cmake_feature_test_cxx_reference_qualified_functions )
# set(_cmake_feature_test_cxx_inheriting_constructors )
# set(_cmake_feature_test_cxx_alignas )
# set(_cmake_feature_test_cxx_alignof )
# set(_cmake_feature_test_cxx_thread_local )
# set(_cmake_feature_test_cxx_func_identifier )
# set(_cmake_feature_test_cxx_sizeof_member )

# set(_cmake_feature_test_cxx_user_literals )
# set(_cmake_feature_test_cxx_unrestricted_unions )
# set(_cmake_feature_test_cxx_unicode_literals )
# set(_cmake_feature_test_cxx_inline_namespaces )
# set(_cmake_feature_test_cxx_constexpr )
# set(_cmake_feature_test_cxx_noexcept )
# set(_cmake_feature_test_cxx_attributes )

set(_oldestSupported)
