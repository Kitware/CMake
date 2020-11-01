project(unitybuild_code_before_and_after_include C)

set(src "${CMAKE_CURRENT_BINARY_DIR}/s1.c")
file(WRITE "${src}" "int s1(void) { return 0; }\n")

add_library(tgt SHARED ${src})

set_target_properties(tgt
  PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_CODE_BEFORE_INCLUDE "#define NOMINMAX"
    UNITY_BUILD_CODE_AFTER_INCLUDE "#undef NOMINMAX"
)
