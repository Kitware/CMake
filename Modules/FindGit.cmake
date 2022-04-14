# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGit
-------

The module defines the following variables:

``GIT_EXECUTABLE``
  Path to Git command-line client.
``Git_FOUND``, ``GIT_FOUND``
  True if the Git command-line client was found.
``GIT_VERSION_STRING``
  The version of Git found.

.. versionadded:: 3.14
  The module defines the following ``IMPORTED`` targets (when
  :prop_gbl:`CMAKE_ROLE` is ``PROJECT``):

``Git::Git``
  Executable of the Git command-line client.

Example usage:

.. code-block:: cmake

   find_package(Git)
   if(Git_FOUND)
     message("Git found: ${GIT_EXECUTABLE}")
   endif()
#]=======================================================================]

# Look for 'git'
#
set(git_names git)

# Prefer .cmd variants on Windows unless running in a Makefile
# in the MSYS shell.
#
if(CMAKE_HOST_WIN32)
  if(NOT CMAKE_GENERATOR MATCHES "MSYS")
    set(git_names git.cmd git)
    # GitHub search path for Windows
    file(GLOB github_path
      "$ENV{LOCALAPPDATA}/Github/PortableGit*/cmd"
      "$ENV{LOCALAPPDATA}/Github/PortableGit*/bin"
      )
    # SourceTree search path for Windows
    set(_git_sourcetree_path "$ENV{LOCALAPPDATA}/Atlassian/SourceTree/git_local/bin")
  endif()
endif()

# First search the PATH and specific locations.
find_program(GIT_EXECUTABLE
  NAMES ${git_names}
  PATHS ${github_path} ${_git_sourcetree_path}
  DOC "Git command line client"
  )

if(CMAKE_HOST_WIN32)
  # Now look for installations in Git/ directories under typical installation
  # prefixes on Windows.  Exclude PATH from this search because VS 2017's
  # command prompt happens to have a PATH entry with a Git/ subdirectory
  # containing a minimal git not meant for general use.
  find_program(GIT_EXECUTABLE
    NAMES ${git_names}
    PATH_SUFFIXES Git/cmd Git/bin
    NO_SYSTEM_ENVIRONMENT_PATH
    DOC "Git command line client"
    )
endif()

mark_as_advanced(GIT_EXECUTABLE)

unset(git_names)
unset(_git_sourcetree_path)

if(GIT_EXECUTABLE)
  # Avoid querying the version if we've already done that this run. For
  # projects that use things like ExternalProject or FetchContent heavily,
  # this saving can be measurable on some platforms.
  #
  # This is an internal property, projects must not try to use it.
  # We don't want this stored in the cache because it might still change
  # between CMake runs, but it shouldn't change during a run for a given
  # git executable location.
  set(__doGitVersionCheck TRUE)
  get_property(__gitVersionProp GLOBAL
    PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  )
  if(__gitVersionProp)
    list(GET __gitVersionProp 0 __gitExe)
    list(GET __gitVersionProp 1 __gitVersion)
    if(__gitExe STREQUAL GIT_EXECUTABLE AND NOT __gitVersion STREQUAL "")
      set(GIT_VERSION_STRING "${__gitVersion}")
      set(__doGitVersionCheck FALSE)
    endif()
    unset(__gitExe)
    unset(__gitVersion)
  endif()
  unset(__gitVersionProp)

  if(__doGitVersionCheck)
    execute_process(COMMAND ${GIT_EXECUTABLE} --version
                    OUTPUT_VARIABLE git_version
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (git_version MATCHES "^git version [0-9]")
      string(REPLACE "git version " "" GIT_VERSION_STRING "${git_version}")
      set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
        "${GIT_EXECUTABLE};${GIT_VERSION_STRING}"
      )
    endif()
    unset(git_version)
  endif()
  unset(__doGitVersionCheck)

  get_property(_findgit_role GLOBAL PROPERTY CMAKE_ROLE)
  if(_findgit_role STREQUAL "PROJECT" AND NOT TARGET Git::Git)
    add_executable(Git::Git IMPORTED)
    set_property(TARGET Git::Git PROPERTY IMPORTED_LOCATION "${GIT_EXECUTABLE}")
  endif()
  unset(_findgit_role)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Git
                                  REQUIRED_VARS GIT_EXECUTABLE
                                  VERSION_VAR GIT_VERSION_STRING)
