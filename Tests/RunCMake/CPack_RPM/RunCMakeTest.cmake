include(RunCPack)

set(RunCPack_GENERATORS RPM)
set(RunCPack_GLOB *.rpm)
set(RunCPack_VERIFY rpm -qlp)

run_cpack(Basic)
