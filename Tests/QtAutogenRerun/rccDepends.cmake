# When a file listed in a .qrc file changes the target must be rebuilt
set(timeformat "%Y%j%H%M%S")
set(rccDependsSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/rccDepends")
set(rccDependsBinDir "${CMAKE_CURRENT_BINARY_DIR}/rccDepends")

# Initial build
configure_file(${rccDependsSrcDir}/res1a.qrc.in ${rccDependsBinDir}/res1.qrc COPYONLY)
configure_file(${rccDependsSrcDir}/res2a.qrc.in ${rccDependsBinDir}/res2.qrc.in COPYONLY)
try_compile(RCC_DEPENDS
  "${rccDependsBinDir}"
  "${rccDependsSrcDir}"
  rccDepends
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
              "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(SEND_ERROR "Initial build of rccDepends failed. Output: ${output}")
endif()
# Get name of the output binary
file(STRINGS "${rccDependsBinDir}/target.txt" targetList ENCODING UTF-8)
list(GET targetList 0 rccDependsBin)

file(TIMESTAMP "${rccDependsBin}" timeBegin "${timeformat}")
# Sleep, touch regular qrc input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${rccDependsBinDir}/res1/input.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Second build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep1 "${timeformat}")
if (NOT timeStep1 GREATER timeBegin)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the first step!")
endif()
# Sleep, update regular qrc file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
configure_file(${rccDependsSrcDir}/res1b.qrc.in ${rccDependsBinDir}/res1.qrc COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Third build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep2 "${timeformat}")
if (NOT timeStep2 GREATER timeStep1)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the second step!")
endif()
# Sleep, touch regular qrc newly added input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${rccDependsBinDir}/res1/inputAdded.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Fourth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep3 "${timeformat}")
if (NOT timeStep3 GREATER timeStep2)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the third step!")
endif()
# Sleep, touch generated qrc input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${rccDependsBinDir}/res2/input.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Fifth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep4 "${timeformat}")
if (NOT timeStep4 GREATER timeStep3)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the fourth step!")
endif()
# Sleep, update generated qrc file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
configure_file(${rccDependsSrcDir}/res2b.qrc.in ${rccDependsBinDir}/res2.qrc.in COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Sixth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep5 "${timeformat}")
if (NOT timeStep5 GREATER timeStep4)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the fitfh step!")
endif()
# Sleep, touch generated qrc newly added input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${rccDependsBinDir}/res2/inputAdded.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${rccDependsBinDir}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Seventh build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep6 "${timeformat}")
if (NOT timeStep6 GREATER timeStep5)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the sixth step!")
endif()
