install(FILES cmake-help.vim cmake-indent.vim cmake-syntax.vim DESTINATION ${CMAKE_DATA_DIR}/editors/vim)
install(FILES cmake-mode.el DESTINATION ${CMAKE_DATA_DIR}/editors/emacs)
add_subdirectory (bash-completion)
