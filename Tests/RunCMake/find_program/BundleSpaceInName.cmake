set(fakeApp "${CMAKE_CURRENT_BINARY_DIR}/Fake app.app/Contents/MacOS/Fake app")
file(WRITE "${fakeApp}" "#!/bin/sh\n")
execute_process(COMMAND chmod a+rx "${fakeApp}")

find_program(FakeApp_EXECUTABLE NAMES "Fake app" NO_DEFAULT_PATH
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
)
message(STATUS "FakeApp_EXECUTABLE='${FakeApp_EXECUTABLE}'")
