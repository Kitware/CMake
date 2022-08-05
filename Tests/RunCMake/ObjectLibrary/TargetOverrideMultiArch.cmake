add_library(A OBJECT a.c)
set_target_properties(A PROPERTIES OSX_ARCHITECTURES "x86_64;arm64")
install(TARGETS A DESTINATION lib)
