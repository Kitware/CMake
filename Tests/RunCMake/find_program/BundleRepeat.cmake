# A non-ASCII directory name checks that the bundle path is kept as given.
set(fakeApp "${CMAKE_CURRENT_BINARY_DIR}/Café/Fake.app/Contents/MacOS/Fake")
file(WRITE "${fakeApp}" "#!/bin/sh\n")
execute_process(COMMAND chmod a+rx "${fakeApp}")

# Look up the same bundle twice in one run.
foreach(var IN ITEMS FakeApp1 FakeApp2)
  find_program(${var} NAMES Fake NO_DEFAULT_PATH NO_CACHE
    PATHS "${CMAKE_CURRENT_BINARY_DIR}/Café"
  )
  if(NOT "${${var}}" STREQUAL "${fakeApp}")
    message(SEND_ERROR "${var}='${${var}}'\nexpected '${fakeApp}'")
  endif()
endforeach()
