
enable_language(C)

add_library(foo STATIC foo.c)
target_sources(foo PUBLIC FILE_SET HEADERS FILES foo.h)
target_sources(foo PRIVATE FILE_SET priv TYPE HEADERS FILES foo.h)
target_sources(foo INTERFACE FILE_SET iface TYPE HEADERS FILES foo.h)

function(check_scope fs expected)
  get_property(scope FILE_SET ${fs} TARGET foo PROPERTY SCOPE)
  if(NOT scope STREQUAL expected)
    message(SEND_ERROR "wrong scope for ${fs}: ${scope} instead of ${expected}")
  endif()
endfunction()

check_scope(HEADERS PUBLIC)
check_scope(priv PRIVATE)
check_scope(iface INTERFACE)
