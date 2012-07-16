
macro(shared_variant_build_pass Source Message Languages)
  build_pass("libshared_and_static.h" "shared_variant" "lib_shared_and_static" "${Source}" ${Message} "${Languages}")
endmacro()

macro(shared_variant_build_fail Source Message)
  build_fail("libshared_and_static.h" "shared_variant" "lib_shared_and_static" "${Source}" ${Message} "${Languages}")
endmacro()

macro(static_variant_build_pass Source Message)
  build_pass("libshared_and_static.h" "static_variant" "lib_shared_and_static" "${Source}" ${Message} "${Languages}")
endmacro()

macro(static_variant_build_fail Source Message)
  build_fail("libshared_and_static.h" "static_variant" "lib_shared_and_static" "${Source}" ${Message} "${Languages}")
endmacro()


macro(run_tests Languages)

  static_variant_build_pass("return libshared_and_static_exported();" "Failed to build static variant\n" "${Languages}")
  shared_variant_build_pass("return libshared_and_static_exported();" "Failed to build shared variant\n" "${Languages}")
  # if (COMPILER_HAS_DEPRECATED)
  #   shared_variant_build_fail("return libshared_and_static_deprecated();" "Built shared deprecated variant")
  #   static_variant_build_fail("return libshared_and_static_deprecated();" "Built static deprecated variant")
  # else()
  #   shared_variant_build_pass("return libshared_and_static_deprecated();" "Built shared deprecated variant")
  #   static_variant_build_pass("return libshared_and_static_deprecated();" "Built static deprecated variant")
  # endif()
  static_variant_build_pass("return libshared_and_static_not_exported();" "Failed to build static not exported variant\n" "${Languages}")

  if (WIN32 OR COMPILER_HAS_HIDDEN_VISIBILITY)
    shared_variant_build_fail("return libshared_and_static_not_exported();" "Built shared not exported variant\n" "${Languages}")
  else()
    shared_variant_build_pass("return libshared_and_static_not_exported();" "Built shared not exported variant" "${Languages}")
  endif()
endmacro()

run_tests("")
run_tests("CXX")
run_tests("C")
run_tests("C CXX")
