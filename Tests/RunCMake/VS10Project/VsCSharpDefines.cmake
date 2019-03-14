enable_language(CSharp)

add_library(foo SHARED
  foo.cs)

set_target_properties(foo PROPERTIES
  LINKER_LANGUAGE CSharp)


# Issue 18698
target_compile_definitions(
  foo
    PUBLIC
      MY_FOO_DEFINE
      "MY_BAR_ASSIGNMENT=bar"
      $<$<CONFIG:Debug>:DEFINE_ONLY_FOR_DEBUG>
      $<$<CONFIG:Release>:DEFINE_ONLY_FOR_RELEASE>
)
