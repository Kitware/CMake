# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindDLLTOOL
-----------

Find the ``dlltool`` executable.

Input Variables
^^^^^^^^^^^^^^^

``DLLTOOL_NAME_HINTS``
  Specify the file names of the ``dlltool`` program to be searched for.
  The variable defaults to `dlltool`.

Output Variables
^^^^^^^^^^^^^^^^

``DLLTOOL_EXECUTABLE``
  path to the ``dlltool`` program if found

``DLLTOLL_VERSION``
  version of ``dlltool`` if found

``DLLTOOL_FOUND``
  "True" if the program was found

#]=======================================================================]

find_program (DLLTOOL_EXECUTABLE
              NAMES ${DLLTOOL_NAME_HINTS} dlltool
              DOC   "path to the dlltool executable")

set (_dlltool_REQUIRED_VARS "DLLTOOL_EXECUTABLE")

if (DLLTOOL_EXECUTABLE)

  set (_dlltool_SAVED_LC_ALL "$ENV{LC_ALL}")
  set (ENV{LC_ALL} C)

  execute_process (COMMAND           ${DLLTOOL_EXECUTABLE} --version
                   OUTPUT_VARIABLE   DLLTOOL_version_output
                   ERROR_VARIABLE    DLLTOOL_version_error
                   RESULT_VARIABLE   DLLTOOL_version_result
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

  set (ENV{LC_ALL} ${_dlltool_SAVED_LC_ALL})
  unset (_dlltool_SAVED_LC_ALL)

  if (NOT ${DLLTOOL_version_result} EQUAL 0)
    message (SEND_ERROR
        "Command '${DLLTOOL_EXECUTABLE} --version' failed with output:\n"
        "${DLLTOOL_version_error}.")
  elseif ("${DLLTOOL_version_output}" MATCHES
         "^GNU[ \t]+.*dlltool.*[ \t]+\\(GNU Binutils\\)[ \t]+([^ \t\r\n]+)\r?\n")
    set (DLLTOOL_VERSION "${CMAKE_MATCH_1}")
    mark_as_advanced (DLLTOOL_EXECUTABLE)
  endif ()

elseif (DLLTOOL_FIND_REQUIRED AND NOT DLLTOOL_NAME_HINTS)

    list (APPEND _dlltool_REQUIRED_VARS "DLLTOOL_NAME_HINTS")
    message (SEND_ERROR
              "\n"
              "No dlltool with the default name 'dlltool' was found.  "
              "Define the variable 'DLLTOOL_NAME_HINTS' for better finding results.\n")

endif ()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args (DLLTOOL
                                   REQUIRED_VARS  ${_dlltool_REQUIRED_VARS}
                                   VERSION_VAR    DLLTOOL_VERSION)
unset (_dlltool_REQUIRED_VARS)
