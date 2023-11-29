# Cross-compiling is normally global.  Cover it without duplicating everything.
set(CMAKE_CROSSCOMPILING 1)
add_executable(cxx_exe_cross_emulator ../../empty.cxx)
set_property(TARGET cxx_exe_cross_emulator PROPERTY CROSSCOMPILING_EMULATOR no-such-emulator)

add_executable(cxx_exe_cross_emulator_args ../../empty.cxx)
set_property(TARGET cxx_exe_cross_emulator_args PROPERTY CROSSCOMPILING_EMULATOR "no-such-emulator;arg1;arg2 with space")

add_executable(cxx_exe_test_launcher_and_cross_emulator ../../empty.cxx)
set_property(TARGET cxx_exe_test_launcher_and_cross_emulator PROPERTY TEST_LAUNCHER "no-such-launcher")
set_property(TARGET cxx_exe_test_launcher_and_cross_emulator PROPERTY CROSSCOMPILING_EMULATOR "no-such-emulator")
