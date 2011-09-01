
macro(shared_variant_build_pass Source Message)
  build_pass("libshared_and_static.h" "shared_variant" "lib_shared_and_static" "${Source}" ${Message})
endmacro()

macro(shared_variant_build_fail Source Message)
  build_fail("libshared_and_static.h" "shared_variant" "lib_shared_and_static" "${Source}" ${Message})
endmacro()

macro(static_variant_build_pass Source Message)
  build_pass("libshared_and_static.h" "static_variant" "lib_shared_and_static" "${Source}" ${Message})
endmacro()

macro(static_variant_build_fail Source Message)
  build_fail("libshared_and_static.h" "static_variant" "lib_shared_and_static" "${Source}" ${Message})
endmacro()

static_variant_build_pass("return libshared_and_static_exported();" "Failed to build static variant")
shared_variant_build_pass("return libshared_and_static_exported();" "Failed to build shared variant")
# if (COMPILER_HAS_DEPRECATED)
#   shared_variant_build_fail("return libshared_and_static_deprecated();" "Built shared deprecated variant")
#   static_variant_build_fail("return libshared_and_static_deprecated();" "Built static deprecated variant")
# else()
#   shared_variant_build_pass("return libshared_and_static_deprecated();" "Built shared deprecated variant")
#   static_variant_build_pass("return libshared_and_static_deprecated();" "Built static deprecated variant")
# endif()
static_variant_build_pass("return libshared_and_static_not_exported();" "Failed to build static not exported variant")

if (WIN32 OR COMPILER_HAS_HIDDEN_VISIBILITY)
  shared_variant_build_fail("return libshared_and_static_not_exported();" "Built shared not exported variant")
else()
  shared_variant_build_pass("return libshared_and_static_not_exported();" "Built shared not exported variant")
endif()
