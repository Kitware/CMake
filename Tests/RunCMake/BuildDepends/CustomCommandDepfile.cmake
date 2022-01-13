cmake_policy(SET CMP0116 NEW)
enable_language(C)

add_custom_command(
  OUTPUT topcc.c
  DEPFILE topcc_$<CONFIG>.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=${CMAKE_CURRENT_BINARY_DIR}/topcc.c -DINFILE=topccdep.txt -DDEPFILE=topcc_$<CONFIG>.c.d -P "${CMAKE_CURRENT_LIST_DIR}/WriteDepfile.cmake"
  )
add_custom_target(topcc ALL DEPENDS topcc.c)

add_custom_command(
  OUTPUT topexe.c
  DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/topexe_$<CONFIG>.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=topexe.c "-DINFILE=${CMAKE_CURRENT_BINARY_DIR}/topexedep.txt" -DDEPFILE=topexe_$<CONFIG>.c.d -P "${CMAKE_CURRENT_LIST_DIR}/WriteDepfile.cmake"
  )
add_executable(topexe "${CMAKE_CURRENT_BINARY_DIR}/topexe.c")

add_custom_command(
  OUTPUT toplib.c
  DEPFILE toplib.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=toplib.c -DINFILE=toplibdep.txt -DDEPFILE=toplib.c.d -P "${CMAKE_CURRENT_LIST_DIR}/WriteDepfile.cmake"
  )
add_library(toplib STATIC toplib.c)

add_subdirectory(DepfileSubdir)

add_custom_command(
  OUTPUT toplib2.c
  DEPFILE toplib2.c.d
    COMMAND ${CMAKE_COMMAND} -DOUTFILE=toplib2.c -DINFILE=toplibdep2.txt -DDEPFILE=toplib2.c.d -P "${CMAKE_CURRENT_LIST_DIR}/WriteDepfile2.cmake"
  )
add_library(toplib2 STATIC toplib2.c)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
function(check_exists file)
  if(NOT EXISTS \"\${file}\")
    string(APPEND RunCMake_TEST_FAILED \"\${file} does not exist\\n\")
  endif()
  set(RunCMake_TEST_FAILED \"\${RunCMake_TEST_FAILED}\" PARENT_SCOPE)
endfunction()

function(check_not_exists file)
  if(EXISTS \"\${file}\")
    string(APPEND RunCMake_TEST_FAILED \"\${file} exists\\n\")
  endif()
  set(RunCMake_TEST_FAILED \"\${RunCMake_TEST_FAILED}\" PARENT_SCOPE)
endfunction()

set(check_pairs
  \"${CMAKE_BINARY_DIR}/topcc.c|${CMAKE_BINARY_DIR}/topccdep.txt\"
  \"$<TARGET_FILE:topexe>|${CMAKE_BINARY_DIR}/topexedep.txt\"
  \"$<TARGET_FILE:toplib>|${CMAKE_BINARY_DIR}/toplibdep.txt\"
  \"$<TARGET_FILE:toplib2>|${CMAKE_BINARY_DIR}/toplibdep2.txt\"
  \"${CMAKE_BINARY_DIR}/DepfileSubdir/subcc.c|${CMAKE_BINARY_DIR}/DepfileSubdir/subccdep.txt\"
  \"$<TARGET_FILE:subexe>|${CMAKE_BINARY_DIR}/DepfileSubdir/subexedep.txt\"
  \"$<TARGET_FILE:sublib>|${CMAKE_BINARY_DIR}/DepfileSubdir/sublibdep.txt\"
  )

if(check_step EQUAL 3)
  list(APPEND check_pairs
    \"${CMAKE_BINARY_DIR}/step3.timestamp|${CMAKE_BINARY_DIR}/topcc.c\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|$<TARGET_FILE:topexe>\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|$<TARGET_FILE:toplib>\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|$<TARGET_FILE:toplib2>\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|${CMAKE_BINARY_DIR}/DepfileSubdir/subcc.c\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|$<TARGET_FILE:subexe>\"
    \"${CMAKE_BINARY_DIR}/step3.timestamp|$<TARGET_FILE:sublib>\"
    )
endif()
")
