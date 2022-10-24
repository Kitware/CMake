include(GetPrerequisites)

function(check_script script)
  set(prereqs "")
  get_prerequisites(${script} prereqs 1 1 "" "")
  if(NOT "${prereqs}" STREQUAL "")
    message(FATAL_ERROR "Prerequisites for ${script} not empty")
  endif()
endfunction()

# Should not throw any errors
# Regular executable
if(SAMPLE_EXE)
  get_prerequisites("${SAMPLE_EXE}" cmake_prereqs 1 1 "" "")
endif()
# Shell script
check_script(${CMAKE_CURRENT_LIST_DIR}/script.sh)
# Batch script
check_script(${CMAKE_CURRENT_LIST_DIR}/script.bat)
# Shell script without extension
check_script(${CMAKE_CURRENT_LIST_DIR}/script)
