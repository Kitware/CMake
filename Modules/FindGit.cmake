# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindGit
# -------
#
# The module defines the following variables:
#
# ``GIT_EXECUTABLE``
#   Path to Git command-line client.
# ``Git_FOUND``, ``GIT_FOUND``
#   True if the Git command-line client was found.
# ``GIT_VERSION_STRING``
#   The version of Git found.
#
# Example usage:
#
# .. code-block:: cmake
#
#    find_package(Git)
#    if(Git_FOUND)
#      message("Git found: ${GIT_EXECUTABLE}")
#    endif()
#
# For convenience, the package also provides the following functions:
#
# ::
#
#    git_get_revision(VARNAME [WORKING_DIRECTORY])
#    git_get_branch(VARNAME [WORKING_DIRECTORY])
#
# Both functions return the current Git revision (full ID) and branch name, respectively.
# If the functions are invoked but Git was not found, the configure step stops with an error.
# The arguments are
#
# ``VARNAME``
#   Name of the variable to contain the result.
#
# ``WORKING_DIRECTORY``
#   The working directory at which to execute the git commands.
#   If not specified, :variable:`CMAKE_CURRENT_SOURCE_DIR` is assumed.

# Look for 'git' or 'eg' (easy git)
#
set(git_names git eg)

# Prefer .cmd variants on Windows unless running in a Makefile
# in the MSYS shell.
#
if(CMAKE_HOST_WIN32)
  if(NOT CMAKE_GENERATOR MATCHES "MSYS")
    set(git_names git.cmd git eg.cmd eg)
    # GitHub search path for Windows
    file(GLOB github_path
      "$ENV{LOCALAPPDATA}/Github/PortableGit*/cmd"
      "$ENV{LOCALAPPDATA}/Github/PortableGit*/bin"
      )
    # SourceTree search path for Windows
    set(_git_sourcetree_path "$ENV{LOCALAPPDATA}/Atlassian/SourceTree/git_local/bin")
  endif()
endif()

find_program(GIT_EXECUTABLE
  NAMES ${git_names}
  PATHS ${github_path} ${_git_sourcetree_path}
  PATH_SUFFIXES Git/cmd Git/bin
  DOC "Git command line client"
  )
mark_as_advanced(GIT_EXECUTABLE)

unset(git_names)
unset(_git_sourcetree_path)

if(GIT_EXECUTABLE)
  execute_process(COMMAND ${GIT_EXECUTABLE} --version
                  OUTPUT_VARIABLE git_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (git_version MATCHES "^git version [0-9]")
    string(REPLACE "git version " "" GIT_VERSION_STRING "${git_version}")
  endif()
  unset(git_version)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Git
                                  REQUIRED_VARS GIT_EXECUTABLE
                                  VERSION_VAR GIT_VERSION_STRING)

# Convenience Git repo & branch information functions
# Added by Daniel Wirtz<daniel.wirtz@simtech.uni-stuttgart.de>
function(git_get_revision VARNAME)
    if (NOT GIT_FOUND)
        message(FATAL_ERROR "Cannot use git_get_revision: Git was not found.")
    endif()
    set(WD ${ARGV1}) 
    if("${WD}" STREQUAL "")
        set(WD ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --max-count=1 HEAD
        OUTPUT_VARIABLE RES
        ERROR_VARIABLE ERR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${WD})
    set(${VARNAME} ${RES} PARENT_SCOPE)
    if (ERR)
        message(WARNING "Issuing Git command '${GIT_EXECUTABLE} rev-list --max-count=1 HEAD' failed: ${ERR}")
    endif()
endfunction()

function(git_get_branch VARNAME)
    if (NOT GIT_FOUND)
        message(FATAL_ERROR "Cannot use git_get_branch: Git was not found.")
    endif()
    set(WD ${ARGV1}) 
    if("${WD}" STREQUAL "")
        set(WD ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --all
        OUTPUT_VARIABLE RES
        ERROR_VARIABLE ERR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${WD})
    if (ERR)
        message(WARNING "Issuing Git command '${GIT_EXECUTABLE} describe --all' failed: ${ERR}")
    endif()
    set(${VARNAME} ${RES} PARENT_SCOPE)
endfunction()                                  