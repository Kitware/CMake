enable_language(C)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(test_launcher test_launcher.c)

add_executable(launcher_test launcher_test.c)
xcode_sign_adhoc(launcher_test)
set(launcher
  "$<TARGET_FILE:test_launcher>"
  ""   # Verify that an empty list item will be preserved
  "launcherparam"
  "--"
)
set_property(TARGET launcher_test PROPERTY TEST_LAUNCHER "${launcher}")
set(emulator
  "$<TARGET_FILE:test_launcher>"
  ""   # Verify that an empty list item will be preserved
  "emulatorparam"
  "--"
)
set_property(TARGET launcher_test PROPERTY CROSSCOMPILING_EMULATOR "${emulator}")

gtest_discover_tests(
  launcher_test
)
