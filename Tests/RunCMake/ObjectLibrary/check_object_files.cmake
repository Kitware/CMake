
if (NOT TARGET_OBJECTS)
  message(SEND_ERROR "Object not passed as -DTARGET_OBJECTS")
endif()

foreach(objlib_file IN LISTS objects)
  message(STATUS "objlib_file: =${objlib_file}=")

  set(file_exists False)
  if (EXISTS "${objlib_file}")
    set(file_exists True)
  endif()

  if (NOT file_exists)
    message(SEND_ERROR "File \"${objlib_file}\" does not exist!${tried}")
  endif()
endforeach()
