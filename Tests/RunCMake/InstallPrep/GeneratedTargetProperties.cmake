include(GeneratedCommon.cmake)

# Each property uses its own target so that preparing one file cannot
# produce another as a side effect.
foreach(property IN ITEMS PUBLIC_HEADER PRIVATE_HEADER RESOURCE)
  string(TOLOWER "${property}" name)
  add_generated_file(generated_file "generated_${name}.h")
  add_library(generated_${name} STATIC EXCLUDE_FROM_ALL
    "${generated_target_source}" "${generated_file}")
  set_property(TARGET generated_${name}
    PROPERTY ${property} "${generated_file}")
  install(TARGETS generated_${name}
    ARCHIVE DESTINATION lib
      COMPONENT Archive
      EXCLUDE_FROM_ALL
    ${property} DESTINATION include/${name}
      COMPONENT Headers
  )
endforeach()

# Public headers that are not generated do not need their target built.
add_library(plain_headers STATIC EXCLUDE_FROM_ALL neverbuild.c plain.h)
set_property(TARGET plain_headers PROPERTY PUBLIC_HEADER plain.h)
install(TARGETS plain_headers
  ARCHIVE DESTINATION lib
    COMPONENT Archive
    EXCLUDE_FROM_ALL
  PUBLIC_HEADER DESTINATION include
    COMPONENT Headers
)

add_library(unit_tests STATIC neverbuild.c)
