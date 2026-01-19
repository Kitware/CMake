enable_language(CXX)
set(CMAKE_CXX_PVS_STUDIO "${PSEUDO_PVS};-bad")
add_executable(main main.cxx)
