enable_language(C)

add_library(colon_dependency STATIC EXCLUDE_FROM_ALL lib.c)
install(TARGETS colon_dependency
  ARCHIVE DESTINATION lib
    COMPONENT "Colon:Name"
)

# Components whose names are not representable get no preparation target,
# which is diagnosed when they have build dependencies.
add_library(unsafe_dependency STATIC EXCLUDE_FROM_ALL lib.c)
foreach(component IN ITEMS "Unsafe/../Name" "Empty//Segment" "./Dot")
  install(TARGETS unsafe_dependency
    ARCHIVE DESTINATION lib
      COMPONENT "${component}"
      EXCLUDE_FROM_ALL
  )
endforeach()

# Rules in a component named "all" contribute to install_prep/all only when
# they are part of a full installation.
add_library(all_component_included STATIC EXCLUDE_FROM_ALL lib.c)
install(TARGETS all_component_included
  ARCHIVE DESTINATION lib
    COMPONENT all
)

# An excluded rule in a component named "all" is prepared by nothing, which
# is diagnosed.
add_library(all_component_excluded STATIC EXCLUDE_FROM_ALL neverbuild.c)
install(TARGETS all_component_excluded
  ARCHIVE DESTINATION lib
    COMPONENT all
    EXCLUDE_FROM_ALL
)

add_library(unit_tests STATIC neverbuild.c)
