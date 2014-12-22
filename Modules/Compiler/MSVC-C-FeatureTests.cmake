
# Reference: http://msdn.microsoft.com/en-us/library/vstudio/hh567368.aspx
# http://blogs.msdn.com/b/vcblog/archive/2013/06/28/c-11-14-stl-features-fixes-and-breaking-changes-in-vs-2013.aspx
# http://blogs.msdn.com/b/vcblog/archive/2014/11/17/c-11-14-17-features-in-vs-2015-preview.aspx
# http://www.visualstudio.com/en-us/news/vs2015-preview-vs.aspx

set(_cmake_oldestSupported "_MSC_VER >= 1300")

set(MSVC_2010 "_MSC_VER >= 1600")
set(_cmake_feature_test_c_static_assert "${MSVC_2010}")
set(_cmake_feature_test_c_variadic_macros "${MSVC_2010}")

set(MSVC_2003 "_MSC_VER >= 1300")
set(_cmake_feature_test_c_function_prototypes "${MSVC_2003}")

# Currently unsupported:
# restrict requires the __restrict syntax in msvc
# set(_cmake_feature_test_c_restrict)

# Unset all the variables that we don't need exposed.
# _cmake_oldestSupported is required by WriteCompilerDetectionHeader
set(MSVC_2010)
set(MSVC_2003)