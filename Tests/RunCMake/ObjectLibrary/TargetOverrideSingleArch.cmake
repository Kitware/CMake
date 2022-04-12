add_library(A OBJECT a.c)
set_target_properties(A PROPERTIES OSX_ARCHITECTURES ${osx_arch})
install(TARGETS A DESTINATION lib)
