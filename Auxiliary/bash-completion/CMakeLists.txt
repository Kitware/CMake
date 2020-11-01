# We need to integrate into the system install, or this will silently fail to
# accomplish anything at all, and packagers won't even know it exists. Use the
# `<sharedir>/bash-completion/completions/` hierarchy by default, rooted in
# CMake's XDGDATA_DIR definition of the sharedir. This works with installation
# to `/usr` or `/usr/local` (or any prefix which bash-completion is configured
# with) as well as a simple installation by a local user into their home
# directory *if* the prefix is `$HOME/.local` since `.local/share/` is part of
# the bash-completion search path too.
# For more complex installations, packagers can set CMake_INSTALL_BASH_COMP_DIR
# to another system location.

if(NOT CMake_INSTALL_BASH_COMP_DIR)
  if(CMAKE_BASH_COMP_DIR)
    # Honor previous customization option.
    set(CMake_INSTALL_BASH_COMP_DIR "${CMAKE_BASH_COMP_DIR}")
  else()
    # Default.
    set(CMake_INSTALL_BASH_COMP_DIR ${CMAKE_XDGDATA_DIR}/bash-completion/completions)
  endif()
endif()
install(FILES cmake cpack ctest DESTINATION ${CMake_INSTALL_BASH_COMP_DIR})
