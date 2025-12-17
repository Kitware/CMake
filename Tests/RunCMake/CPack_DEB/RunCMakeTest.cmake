include(RunCPack)

set(RunCPack_GENERATORS DEB)
set(RunCPack_GLOB *.deb)
set(RunCPack_VERIFY ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/verify.cmake --)

run_cpack(Basic)
