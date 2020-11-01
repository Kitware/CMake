# Install Vim files to a typical system integration directory.
# Packagers can set CMake_INSTALL_VIMFILES_DIR to control this.
if(NOT CMake_INSTALL_VIMFILES_DIR)
  set(CMake_INSTALL_VIMFILES_DIR ${CMAKE_XDGDATA_DIR}/vim/vimfiles)
endif()
install(DIRECTORY vim/indent vim/syntax DESTINATION ${CMake_INSTALL_VIMFILES_DIR})

# Install Emacs files to a typical system integration directory.
# Packagers can set CMake_INSTALL_EMACS_DIR to control this.
if(NOT CMake_INSTALL_EMACS_DIR)
  set(CMake_INSTALL_EMACS_DIR ${CMAKE_XDGDATA_DIR}/emacs/site-lisp)
endif()
install(FILES cmake-mode.el DESTINATION ${CMake_INSTALL_EMACS_DIR})

install(FILES cmake.m4 DESTINATION ${CMAKE_XDGDATA_DIR}/aclocal)
add_subdirectory (bash-completion)
