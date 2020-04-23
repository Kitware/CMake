install(DIRECTORY vim/indent vim/syntax DESTINATION ${CMAKE_XDGDATA_DIR}/vim/vimfiles)
install(FILES cmake-mode.el DESTINATION ${CMAKE_XDGDATA_DIR}/emacs/site-lisp)
install(FILES cmake.m4 DESTINATION ${CMAKE_XDGDATA_DIR}/aclocal)
add_subdirectory (bash-completion)
