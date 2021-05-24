# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindMsys
--------

.. versionadded:: 3.21

Find MSYS, a POSIX-compatible environment that runs natively
on Microsoft Windows
#]=======================================================================]

if (WIN32)
  if(MSYS_INSTALL_PATH)
    set(MSYS_CMD "${MSYS_INSTALL_PATH}/msys2_shell.cmd")
  endif()

  find_program(MSYS_CMD
    NAMES msys2_shell.cmd
    PATHS
      "C:/msys64"
      "C:/msys32"
      "C:/MSYS"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MSYS\\setup;rootdir]"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygnus Solutions\\MSYS\\mounts v2\\/;native]"
  )
  get_filename_component(MSYS_INSTALL_PATH "${MSYS_CMD}" DIRECTORY)
  mark_as_advanced(MSYS_CMD)

endif ()
