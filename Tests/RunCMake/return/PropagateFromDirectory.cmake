
set(VAR1 "initial")
set(VAR2 "initial")

add_subdirectory(subdir)

if((NOT DEFINED VAR1 OR NOT VAR1 STREQUAL "set")
    OR DEFINED VAR2)
  message(SEND_ERROR "erroneous propagation for FUNC1")
endif()
