string(REGEX REPLACE "^/(.*)" "\\1" REL_CMAKE_DATA_DIR "${CMAKE_DATA_DIR}")
install(FILES cmake-help.vim cmake-indent.vim cmake-syntax.vim DESTINATION ${REL_CMAKE_DATA_DIR}/editors/vim)
install(FILES cmake-mode.el DESTINATION ${REL_CMAKE_DATA_DIR}/editors/emacs)
ADD_SUBDIRECTORY (bash-completion)
