include(RunCMake)

run_cmake(CREATE_LINK)
run_cmake(CREATE_LINK-COPY_ON_ERROR)
run_cmake(CREATE_LINK-noarg)
run_cmake(CREATE_LINK-noexist)

if(NOT WIN32
    AND NOT MSYS # FIXME: This works on CYGWIN but not on MSYS
    )
  run_cmake(CREATE_LINK-SYMBOLIC)
  run_cmake(CREATE_LINK-SYMBOLIC-noexist)
endif()
