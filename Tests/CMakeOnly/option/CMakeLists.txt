cmake_minimum_required (VERSION 2.8)
project(OptionTest NONE)

#Verify that normal variable take precedence over cache variables
option(OPT_LOCAL_VAR1 "TEST_VAR" ON)
set(OPT_LOCAL_VAR1 FALSE)
if(OPT_LOCAL_VAR1)
  message(FATAL_ERROR "local variable didn't take precedence over cache variable")
endif()

#Verify that option overwrites existing normal variable
set(OPT_LOCAL_VAR2 FALSE)
option(OPT_LOCAL_VAR2 "TEST_VAR" ON)
if(NOT OPT_LOCAL_VAR2)
  message(FATAL_ERROR "option failed to overwrite existing normal variable")
endif()
