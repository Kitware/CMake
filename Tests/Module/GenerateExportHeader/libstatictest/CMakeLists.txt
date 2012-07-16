
macro(static_build_pass Source Message)
  build_pass("libstatic.h" "libstatic" "libstatic" "${Source}" ${Message} "")
endmacro()

macro(static_build_fail Source Message)
  build_fail("libstatic.h" "libstatic" "libstatic" "${Source}" ${Message} "")
endmacro()

static_build_pass("Libstatic l; return l.libstatic_exported();" "Failed to build exported.")

# if (COMPILER_HAS_DEPRECATED)
#   static_build_fail("Libstatic l; return l.libstatic_deprecated();" "Built use of deprecated class method. This should not be possible.")
#   static_build_fail("libstatic_deprecated();" "Built use of deprecated function. This should not be possible.")
# else()
#   static_build_pass("Libstatic l; return l.libstatic_deprecated();" "Built use of deprecated class method. This should not be possible.")
#   static_build_pass("libstatic_deprecated();" "Built use of deprecated function. This should not be possible.")
# endif()
