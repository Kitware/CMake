set(CMAKE_SKIP_INSTALL_PREP_TARGETS ON)

enable_language(C)

add_library(install_prep_gate_on STATIC lib.c)
install(TARGETS install_prep_gate_on
  ARCHIVE DESTINATION lib
)
