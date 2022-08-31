# Change the executable suffix that try_compile will use for
# COPY_FILE but not inside the test project, to verify
# we can handle envs that try and break everything
get_property(in_try_compile GLOBAL PROPERTY IN_TRY_COMPILE)
if(NOT in_try_compile)
  set(CMAKE_EXECUTABLE_SUFFIX .missing)
endif()
