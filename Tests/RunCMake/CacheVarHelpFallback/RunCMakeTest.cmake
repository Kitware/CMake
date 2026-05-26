include(RunCMake)

# All cases below exercise ``cmake::ProcessCacheArg``: when a built-in
# cache variable is set via ``-D <var>=<value>`` with no ``:TYPE=`` and
# no help text, ``ProcessCacheArg`` consults the built-in documentation
# table (``cmCacheDocumentationTable`` and ``cmCachePatternTable``,
# seeded from ``Help/variable/*.rst`` and the per-language pattern
# manuals) and installs the table's Summary as the entry's HELPSTRING.
# The cache entry's type and value are left untouched.
function(run_define_case case)
  set(RunCMake-check-file CheckCache.cmake)
  run_cmake(${case})
endfunction()

# Bare ``-D <var>=<value>`` of a documented built-in: HELPSTRING is
# populated from the table's Summary; type stays UNINITIALIZED because
# the user did not pin one.
set(RunCMake_TEST_OPTIONS "-DCMAKE_COMPILE_WARNING_AS_ERROR=ON")
run_define_case(DefineSetsTypeAndHelp)
unset(RunCMake_TEST_OPTIONS)

# ``-D <var>:STRING=<value>`` of a documented built-in: user's explicit
# type pin is honored; HELPSTRING is still populated from the table.
set(RunCMake_TEST_OPTIONS "-DCMAKE_COMPILE_WARNING_AS_ERROR:STRING=ON")
run_define_case(DefineTypePreserved)
unset(RunCMake_TEST_OPTIONS)

# Bare ``-D <var>=<value>`` of a project-specific name: no table match,
# so the entry stays ``:UNINITIALIZED=`` with the legacy sentinel
# HELPSTRING preserved for backwards compatibility.
set(RunCMake_TEST_OPTIONS "-DMY_CUSTOM_VAR_XYZZY=x")
run_define_case(DefineUnknownVarUninitialized)
unset(RunCMake_TEST_OPTIONS)

# A built-in variable name absent from the exact-match table but present
# in the pattern table (``CMAKE_<LANG>_CLANG_TIDY``): HELPSTRING comes
# from the pattern's Summary; type stays UNINITIALIZED.
set(RunCMake_TEST_OPTIONS "-DCMAKE_CXX_CLANG_TIDY=clang-tidy")
run_define_case(PatternCxxClangTidy)
unset(RunCMake_TEST_OPTIONS)

# A built-in variable name matching a pattern whose manual documents a
# per-language restriction CMake does not enforce at match time
# (cppcheck applies only to C/CXX, but here we set HIP).  The matcher
# is intentionally permissive; the restriction text travels in the
# Summary prose.
set(RunCMake_TEST_OPTIONS "-DCMAKE_HIP_CPPCHECK=anything")
run_define_case(PatternHipCppcheck)
unset(RunCMake_TEST_OPTIONS)
