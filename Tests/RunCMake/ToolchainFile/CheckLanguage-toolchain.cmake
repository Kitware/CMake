get_property(in_try_compile GLOBAL PROPERTY IN_TRY_COMPILE)
if(in_try_compile)
  message(FATAL_ERROR "Toolchain file included")
endif()
