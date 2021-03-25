
function(write_unity_build_anon_ns_test_files OUTVAR)
  set(srcs)
  foreach(s RANGE 1 8)
    set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cpp")
    file(WRITE "${src}" "
#ifndef CONFIG_H
#define CONFIG_H
#define MY_ANON_NAMESPACE MY_ANON_ID
#define MY_ANON(Name) MY_ANON_NAMESPACE::Name
#define MY_ANON_USING_NAMESPACE using namespace MY_ANON_NAMESPACE
#endif

namespace { namespace MY_ANON_NAMESPACE {
  int i = ${s};
}}
int use_plain_${s}() {
  return MY_ANON_NAMESPACE::i;
}
int func_like_macro_${s}() {
  return MY_ANON(i);
}
int using_macro_${s}() {
  MY_ANON_USING_NAMESPACE;
  return i;
}
")
    list(APPEND srcs "${src}")
  endforeach()
  set(${OUTVAR} ${srcs} PARENT_SCOPE)
endfunction()
