install(FILES cmake-mode.el DESTINATION ${CMAKE_DATA_DIR}/editors/emacs)
install(FILES cmake.m4 DESTINATION share/aclocal)
add_subdirectory (bash-completion)
