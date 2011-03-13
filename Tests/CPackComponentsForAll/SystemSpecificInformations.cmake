
# define a set of string with may-be useful readable name
# this file is meant to be included in a CMakeLists.txt
# not as a standalone CMake script
set(SPECIFIC_COMPILER_NAME "")
set(SPECIFIC_SYSTEM_VERSION_NAME "")
set(SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR "")

# In the WIN32 case try to guess a "readable system name"
if(WIN32)
  set(SPECIFIC_SYSTEM_PREFERED_PACKAGE "NSIS")
  # information taken from
  # http://www.codeguru.com/cpp/w-p/system/systeminformation/article.php/c8973/
  # Win9x series
  if(CMAKE_SYSTEM_VERSION MATCHES "4.0")
    set(SPECIFIC_SYSTEM_VERSION_NAME "Win95")
  endif(CMAKE_SYSTEM_VERSION MATCHES "4.0")
  if(CMAKE_SYSTEM_VERSION MATCHES "4.10")
    set(SPECIFIC_SYSTEM_VERSION_NAME "Win98")
  endif(CMAKE_SYSTEM_VERSION MATCHES "4.10")
  if(CMAKE_SYSTEM_VERSION MATCHES "4.90")
    set(SPECIFIC_SYSTEM_VERSION_NAME "WinME")
  endif(CMAKE_SYSTEM_VERSION MATCHES "4.90")

  # WinNTyyy series
  if(CMAKE_SYSTEM_VERSION MATCHES "3.0")
    set(SPECIFIC_SYSTEM_VERSION_NAME "WinNT351")
  endif(CMAKE_SYSTEM_VERSION MATCHES "3.0")
  if(CMAKE_SYSTEM_VERSION MATCHES "4.1")
    set(SPECIFIC_SYSTEM_VERSION_NAME "WinNT4")
  endif(CMAKE_SYSTEM_VERSION MATCHES "4.1")

  # Win2000/XP series
  if(CMAKE_SYSTEM_VERSION MATCHES "5.0")
    set(SPECIFIC_SYSTEM_VERSION_NAME "Win2000")
  endif(CMAKE_SYSTEM_VERSION MATCHES "5.0")
  if(CMAKE_SYSTEM_VERSION MATCHES "5.1")
    set(SPECIFIC_SYSTEM_VERSION_NAME "WinXP")
  endif(CMAKE_SYSTEM_VERSION MATCHES "5.1")
  if(CMAKE_SYSTEM_VERSION MATCHES "5.2")
    set(SPECIFIC_SYSTEM_VERSION_NAME "Win2003")
  endif(CMAKE_SYSTEM_VERSION MATCHES "5.2")

  # WinVista/7 series
  if(CMAKE_SYSTEM_VERSION MATCHES "6.0")
    set(SPECIFIC_SYSTEM_VERSION_NAME "WinVISTA")
  endif(CMAKE_SYSTEM_VERSION MATCHES "6.0")
  if(CMAKE_SYSTEM_VERSION MATCHES "6.1")
    set(SPECIFIC_SYSTEM_VERSION_NAME "Win7")
  endif(CMAKE_SYSTEM_VERSION MATCHES "6.1")

  # Compilers
  # taken from http://predef.sourceforge.net/precomp.html#sec34
  if(MSVC)
    set(SPECIFIC_COMPILER_NAME "MSVC-Unknown-${MSVC_VERSION}")
    if(MSVC_VERSION EQUAL 1200)
      set(SPECIFIC_COMPILER_NAME "MSVC-6.0")
    endif(MSVC_VERSION EQUAL 1200)
    if(MSVC_VERSION EQUAL 1300)
      set(SPECIFIC_COMPILER_NAME "MSVC-7.0")
    endif(MSVC_VERSION EQUAL 1300)
    if(MSVC_VERSION EQUAL 1310)
      set(SPECIFIC_COMPILER_NAME "MSVC-7.1-2003") #Visual Studio 2003
    endif(MSVC_VERSION EQUAL 1310)
    if(MSVC_VERSION EQUAL 1400)
      set(SPECIFIC_COMPILER_NAME "MSVC-8.0-2005") #Visual Studio 2005
    endif(MSVC_VERSION EQUAL 1400)
    if(MSVC_VERSION EQUAL 1500)
      set(SPECIFIC_COMPILER_NAME "MSVC-9.0-2008") #Visual Studio 2008
    endif(MSVC_VERSION EQUAL 1500)
    if(MSVC_VERSION EQUAL 1600)
      set(SPECIFIC_COMPILER_NAME "MSVC-10.0-2010") #Visual Studio 2010
    endif(MSVC_VERSION EQUAL 1600)
  endif(MSVC)
  if(MINGW)
    set(SPECIFIC_COMPILER_NAME "MinGW")
  endif(MINGW)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(SPECIFIC_SYSTEM_VERSION_NAME "${SPECIFIC_SYSTEM_VERSION_NAME}-x86_64")
  endif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
endif(WIN32)

# In the Linux case try to guess the distro name/type
# using either lsb_release program or fallback
# to the content of the /etc/issue file
if(UNIX)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SPECIFIC_SYSTEM_VERSION_NAME "${CMAKE_SYSTEM_NAME}")
    set(SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR "TGZ")
    find_program(LSB_RELEASE_EXECUTABLE lsb_release)
    if(LSB_RELEASE_EXECUTABLEF)
      execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -i
                      OUTPUT_VARIABLE _TMP_LSB_RELEASE_OUTPUT
                      ERROR_QUIET
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX MATCH "Distributor ID:(.*)" DISTRO_ID ${_TMP_LSB_RELEASE_OUTPUT})
      string(STRIP "${CMAKE_MATCH_1}" DISTRO_ID)
      # replace potential space with underscore
      string(REPLACE " " "_" DISTRO_ID "${DISTRO_ID}")
      execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -r
                      OUTPUT_VARIABLE _TMP_LSB_RELEASE_OUTPUT
                      ERROR_QUIET
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX MATCH "Release:(.*)" DISTRO_RELEASE ${_TMP_LSB_RELEASE_OUTPUT})
      string(STRIP "${CMAKE_MATCH_1}" DISTRO_RELEASE)
      execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -c
                      OUTPUT_VARIABLE _TMP_LSB_RELEASE_OUTPUT
                      ERROR_QUIET
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX MATCH "Codename:(.*)" DISTRO_CODENAME ${_TMP_LSB_RELEASE_OUTPUT})
      string(STRIP "${CMAKE_MATCH_1}" DISTRO_CODENAME)
    elseif (EXISTS "/etc/issue")
      set(LINUX_NAME "")
      file(READ "/etc/issue" LINUX_ISSUE)
      # Fedora case
      if(LINUX_ISSUE MATCHES "Fedora")
        string(REGEX MATCH "release ([0-9]+)" FEDORA "${LINUX_ISSUE}")
        set(DISTRO_ID "Fedora")
        set(DISTRO_RELEASE "${CMAKE_MATCH_1}")
        # FIXME can we find that in /etc/issue
        set(DISTRO_CODENAME "")
      endif(LINUX_ISSUE MATCHES "Fedora")
      # Ubuntu case
      if(LINUX_ISSUE MATCHES "Ubuntu")
        string(REGEX MATCH "buntu ([0-9]+\\.[0-9]+)" UBUNTU "${LINUX_ISSUE}")
        set(DISTRO_ID "Ubuntu")
        set(DISTRO_RELEASE "${CMAKE_MATCH_1}")
        # FIXME can we find that in /etc/issue
        set(DISTRO_CODENAME "")
      endif(LINUX_ISSUE MATCHES "Ubuntu")
      # Debian case
      if(LINUX_ISSUE MATCHES "Debian")
        string(REGEX MATCH "Debian .*ux ([0-9]+\\.[0-9]+)"
               DEBIAN "${LINUX_ISSUE}")
        set(DISTRO_ID "Debian")
        set(DISTRO_RELEASE "${CMAKE_MATCH_1}")
        set(DISTRO_CODENAME "")
      endif(LINUX_ISSUE MATCHES "Debian")
      # Open SuSE case
      if(LINUX_ISSUE MATCHES "SUSE")
        string(REGEX MATCH "SUSE ([0-9]+\\.[0-9]+)" SUSE "${LINUX_ISSUE}")
        set(DISTRO_ID "SUSE")
        set(DISTRO_RELEASE "${CMAKE_MATCH_1}")
        set(DISTRO_CODENAME "")
      endif(LINUX_ISSUE MATCHES "SUSE")
      # Mandriva case
      # TODO
    endif(LSB_RELEASE_EXECUTABLEF)
    # Now mangle some names
    set(LINUX_NAME "${DISTRO_ID}_${DISTRO_RELEASE}")
    if(DISTRO_ID MATCHES "Fedora|Mandriva|SUSE|OpenSUSE")
      set(SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR "RPM")
    endif(DISTRO_ID MATCHES "Fedora|Mandriva|SUSE|OpenSUSE")
    if(DISTRO_ID MATCHES "Debian|Ubuntu")
      set(SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR "DEB")
    endif(DISTRO_ID MATCHES "Debian|Ubuntu")
    if(LINUX_NAME)
      set(SPECIFIC_SYSTEM_VERSION_NAME "${CMAKE_SYSTEM_NAME}-${LINUX_NAME}")
    endif(LINUX_NAME)
  endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(SPECIFIC_SYSTEM_VERSION_NAME
     "${SPECIFIC_SYSTEM_VERSION_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  set(SPECIFIC_COMPILER_NAME "")
endif(UNIX)