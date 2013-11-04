#.rst:
# FindJNI
# -------
#
# Find JNI java libraries.
#
# This module finds if Java is installed and determines where the
# include files and libraries are.  It also determines what the name of
# the library is.  This code sets the following variables:
#
# ::
#
#   JNI_INCLUDE_DIRS      = the include dirs to use
#   JNI_LIBRARIES         = the libraries to use
#   JNI_FOUND             = TRUE if JNI headers and libraries were found.
#   JAVA_AWT_LIBRARY      = the path to the jawt library
#   JAVA_JVM_LIBRARY      = the path to the jvm library
#   JAVA_INCLUDE_PATH     = the include path to jni.h
#   JAVA_INCLUDE_PATH2    = the include path to jni_md.h
#   JAVA_AWT_INCLUDE_PATH = the include path to jawt.h

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Expand {libarch} occurences to java_libarch subdirectory(-ies) and set ${_var}
macro(java_append_library_directories _var)
    # Determine java arch-specific library subdir
    # Mostly based on openjdk/jdk/make/common/shared/Platform.gmk as of openjdk
    # 1.6.0_18 + icedtea patches. However, it would be much better to base the
    # guess on the first part of the GNU config.guess platform triplet.
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(_java_libarch "amd64" "i386")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
        set(_java_libarch "i386")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^alpha")
        set(_java_libarch "alpha")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
        # Subdir is "arm" for both big-endian (arm) and little-endian (armel).
        set(_java_libarch "arm")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
        # mips* machines are bi-endian mostly so processor does not tell
        # endianess of the underlying system.
        set(_java_libarch "${CMAKE_SYSTEM_PROCESSOR}" "mips" "mipsel" "mipseb")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")
        set(_java_libarch "ppc64" "ppc")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
        set(_java_libarch "ppc")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^sparc")
        # Both flavours can run on the same processor
        set(_java_libarch "${CMAKE_SYSTEM_PROCESSOR}" "sparc" "sparcv9")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(parisc|hppa)")
        set(_java_libarch "parisc" "parisc64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^s390")
        # s390 binaries can run on s390x machines
        set(_java_libarch "${CMAKE_SYSTEM_PROCESSOR}" "s390" "s390x")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^sh")
        set(_java_libarch "sh")
    else()
        set(_java_libarch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    # Append default list architectures if CMAKE_SYSTEM_PROCESSOR was empty or
    # system is non-Linux (where the code above has not been well tested)
    if(NOT _java_libarch OR NOT (CMAKE_SYSTEM_NAME MATCHES "Linux"))
        list(APPEND _java_libarch "i386" "amd64" "ppc")
    endif()

    # Sometimes ${CMAKE_SYSTEM_PROCESSOR} is added to the list to prefer
    # current value to a hardcoded list. Remove possible duplicates.
    list(REMOVE_DUPLICATES _java_libarch)

    foreach(_path ${ARGN})
        if(_path MATCHES "{libarch}")
            foreach(_libarch ${_java_libarch})
                string(REPLACE "{libarch}" "${_libarch}" _newpath "${_path}")
                list(APPEND ${_var} "${_newpath}")
            endforeach()
        else()
            list(APPEND ${_var} "${_path}")
        endif()
    endforeach()
endmacro()

get_filename_component(java_install_version
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit;CurrentVersion]" NAME)

set(JAVA_AWT_LIBRARY_DIRECTORIES
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\${java_install_version};JavaHome]/lib"
  )

file(TO_CMAKE_PATH "$ENV{JAVA_HOME}" _JAVA_HOME)

JAVA_APPEND_LIBRARY_DIRECTORIES(JAVA_AWT_LIBRARY_DIRECTORIES
  ${_JAVA_HOME}/jre/lib/{libarch}
  ${_JAVA_HOME}/jre/lib
  ${_JAVA_HOME}/lib
  ${_JAVA_HOME}
  /usr/lib
  /usr/local/lib
  /usr/lib/jvm/java/lib
  /usr/lib/java/jre/lib/{libarch}
  /usr/lib/jvm/jre/lib/{libarch}
  /usr/local/lib/java/jre/lib/{libarch}
  /usr/local/share/java/jre/lib/{libarch}
  /usr/lib/j2sdk1.4-sun/jre/lib/{libarch}
  /usr/lib/j2sdk1.5-sun/jre/lib/{libarch}
  /opt/sun-jdk-1.5.0.04/jre/lib/{libarch}
  /usr/lib/jvm/java-6-sun/jre/lib/{libarch}
  /usr/lib/jvm/java-1.5.0-sun/jre/lib/{libarch}
  /usr/lib/jvm/java-6-sun-1.6.0.00/jre/lib/{libarch}       # can this one be removed according to #8821 ? Alex
  /usr/lib/jvm/java-6-openjdk/jre/lib/{libarch}
  /usr/lib/jvm/java-1.6.0-openjdk-1.6.0.0/jre/lib/{libarch}        # fedora
  # Debian specific paths for default JVM
  /usr/lib/jvm/default-java/jre/lib/{libarch}
  /usr/lib/jvm/default-java/jre/lib
  /usr/lib/jvm/default-java/lib
  # OpenBSD specific paths for default JVM
  /usr/local/jdk-1.7.0/jre/lib/{libarch}
  /usr/local/jre-1.7.0/lib/{libarch}
  /usr/local/jdk-1.6.0/jre/lib/{libarch}
  /usr/local/jre-1.6.0/lib/{libarch}
  )

set(JAVA_JVM_LIBRARY_DIRECTORIES)
foreach(dir ${JAVA_AWT_LIBRARY_DIRECTORIES})
  set(JAVA_JVM_LIBRARY_DIRECTORIES
    ${JAVA_JVM_LIBRARY_DIRECTORIES}
    "${dir}"
    "${dir}/client"
    "${dir}/server"
    )
endforeach()


set(JAVA_AWT_INCLUDE_DIRECTORIES
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\${java_install_version};JavaHome]/include"
  ${_JAVA_HOME}/include
  /usr/include
  /usr/local/include
  /usr/lib/java/include
  /usr/local/lib/java/include
  /usr/lib/jvm/java/include
  /usr/lib/jvm/java-6-sun/include
  /usr/lib/jvm/java-1.5.0-sun/include
  /usr/lib/jvm/java-6-sun-1.6.0.00/include       # can this one be removed according to #8821 ? Alex
  /usr/lib/jvm/java-6-openjdk/include
  /usr/local/share/java/include
  /usr/lib/j2sdk1.4-sun/include
  /usr/lib/j2sdk1.5-sun/include
  /opt/sun-jdk-1.5.0.04/include
  # Debian specific path for default JVM
  /usr/lib/jvm/default-java/include
  # OpenBSD specific path for default JVM
  /usr/local/jdk-1.7.0/include
  /usr/local/jdk-1.6.0/include
  )

foreach(JAVA_PROG "${JAVA_RUNTIME}" "${JAVA_COMPILE}" "${JAVA_ARCHIVE}")
  get_filename_component(jpath "${JAVA_PROG}" PATH)
  foreach(JAVA_INC_PATH ../include ../java/include ../share/java/include)
    if(EXISTS ${jpath}/${JAVA_INC_PATH})
      set(JAVA_AWT_INCLUDE_DIRECTORIES ${JAVA_AWT_INCLUDE_DIRECTORIES} "${jpath}/${JAVA_INC_PATH}")
    endif()
  endforeach()
  foreach(JAVA_LIB_PATH
    ../lib ../jre/lib ../jre/lib/i386
    ../java/lib ../java/jre/lib ../java/jre/lib/i386
    ../share/java/lib ../share/java/jre/lib ../share/java/jre/lib/i386)
    if(EXISTS ${jpath}/${JAVA_LIB_PATH})
      set(JAVA_AWT_LIBRARY_DIRECTORIES ${JAVA_AWT_LIBRARY_DIRECTORIES} "${jpath}/${JAVA_LIB_PATH}")
    endif()
  endforeach()
endforeach()

if(APPLE)
  if(EXISTS ~/Library/Frameworks/JavaVM.framework)
    set(JAVA_HAVE_FRAMEWORK 1)
  endif()
  if(EXISTS /Library/Frameworks/JavaVM.framework)
    set(JAVA_HAVE_FRAMEWORK 1)
  endif()
  if(EXISTS /System/Library/Frameworks/JavaVM.framework)
    set(JAVA_HAVE_FRAMEWORK 1)
  endif()

  if(JAVA_HAVE_FRAMEWORK)
    if(NOT JAVA_AWT_LIBRARY)
      set (JAVA_AWT_LIBRARY "-framework JavaVM" CACHE FILEPATH "Java Frameworks" FORCE)
    endif()

    if(NOT JAVA_JVM_LIBRARY)
      set (JAVA_JVM_LIBRARY "-framework JavaVM" CACHE FILEPATH "Java Frameworks" FORCE)
    endif()

    if(NOT JAVA_AWT_INCLUDE_PATH)
      if(EXISTS /System/Library/Frameworks/JavaVM.framework/Headers/jawt.h)
        set (JAVA_AWT_INCLUDE_PATH "/System/Library/Frameworks/JavaVM.framework/Headers" CACHE FILEPATH "jawt.h location" FORCE)
      endif()
    endif()

    # If using "-framework JavaVM", prefer its headers *before* the others in
    # JAVA_AWT_INCLUDE_DIRECTORIES... (*prepend* to the list here)
    #
    set(JAVA_AWT_INCLUDE_DIRECTORIES
      ~/Library/Frameworks/JavaVM.framework/Headers
      /Library/Frameworks/JavaVM.framework/Headers
      /System/Library/Frameworks/JavaVM.framework/Headers
      ${JAVA_AWT_INCLUDE_DIRECTORIES}
      )
  endif()
else()
  find_library(JAVA_AWT_LIBRARY jawt
    PATHS ${JAVA_AWT_LIBRARY_DIRECTORIES}
  )
  find_library(JAVA_JVM_LIBRARY NAMES jvm JavaVM
    PATHS ${JAVA_JVM_LIBRARY_DIRECTORIES}
  )
endif()

# add in the include path
find_path(JAVA_INCLUDE_PATH jni.h
  ${JAVA_AWT_INCLUDE_DIRECTORIES}
)

find_path(JAVA_INCLUDE_PATH2 jni_md.h
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH}/win32
  ${JAVA_INCLUDE_PATH}/linux
  ${JAVA_INCLUDE_PATH}/freebsd
  ${JAVA_INCLUDE_PATH}/openbsd
  ${JAVA_INCLUDE_PATH}/solaris
  ${JAVA_INCLUDE_PATH}/hp-ux
  ${JAVA_INCLUDE_PATH}/alpha
)

find_path(JAVA_AWT_INCLUDE_PATH jawt.h
  ${JAVA_INCLUDE_PATH}
)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JNI  DEFAULT_MSG  JAVA_AWT_LIBRARY JAVA_JVM_LIBRARY
                                                    JAVA_INCLUDE_PATH  JAVA_INCLUDE_PATH2 JAVA_AWT_INCLUDE_PATH)

mark_as_advanced(
  JAVA_AWT_LIBRARY
  JAVA_JVM_LIBRARY
  JAVA_AWT_INCLUDE_PATH
  JAVA_INCLUDE_PATH
  JAVA_INCLUDE_PATH2
)

set(JNI_LIBRARIES
  ${JAVA_AWT_LIBRARY}
  ${JAVA_JVM_LIBRARY}
)

set(JNI_INCLUDE_DIRS
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH2}
  ${JAVA_AWT_INCLUDE_PATH}
)

