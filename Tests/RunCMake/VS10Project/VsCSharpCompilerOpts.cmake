enable_language(CSharp)

add_library(foo SHARED
  foo.cs)

set_target_properties(foo PROPERTIES
  LINKER_LANGUAGE CSharp)


# Issue 18878
target_compile_options(foo PRIVATE	"/platform:anycpu" "/nowarn:707,808" "/nowarn:909" )

# Debug only warning disable
set(CMAKE_CSharp_FLAGS_DEBUG "${CMAKE_CSharp_FLAGS_DEBUG} /nowarn:505")
