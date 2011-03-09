# By including this file, all library files listed in the variable
# CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS will be installed with
# INSTALL(PROGRAMS ...) into bin for WIN32 and lib
# for non-WIN32. If CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP is set to TRUE
# before including this file, then the INSTALL command is not called.
# The user can use the variable CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS to use a
# custom install command and install them however they want.
# If it is the MSVC compiler, then the microsoft run
# time libraries will be found and automatically added to the
# CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS, and installed.
# If CMAKE_INSTALL_DEBUG_LIBRARIES is set and it is the MSVC
# compiler, then the debug libraries are installed when available.
# If CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY is set then only the debug
# libraries are installed when both debug and release are available.
# If CMAKE_INSTALL_MFC_LIBRARIES is set then the MFC run time
# libraries are installed as well as the CRT run time libraries.
# If CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION is set then the libraries are
# installed to that directory rather than the default.
# If CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS is NOT set, then this file
# warns about required files that do not exist. You can set this variable to
# ON before including this file to avoid the warning. For example, the Visual
# Studio Express editions do not include the redistributable files, so if you
# include this file on a machine with only VS Express installed, you'll get
# the warning.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

IF(MSVC)
  FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)

  IF(CMAKE_CL_64)
    IF(MSVC_VERSION GREATER 1599)
      # VS 10 and later:
      SET(CMAKE_MSVC_ARCH x64)
    ELSE()
      # VS 9 and earlier:
      SET(CMAKE_MSVC_ARCH amd64)
    ENDIF()
  ELSE(CMAKE_CL_64)
    SET(CMAKE_MSVC_ARCH x86)
  ENDIF(CMAKE_CL_64)

  GET_FILENAME_COMPONENT(devenv_dir "${CMAKE_MAKE_PROGRAM}" PATH)
  GET_FILENAME_COMPONENT(base_dir "${devenv_dir}/../.." ABSOLUTE)

  IF(MSVC70)
    SET(__install__libs
      "${SYSTEMROOT}/system32/msvcp70.dll"
      "${SYSTEMROOT}/system32/msvcr70.dll"
      )
  ENDIF(MSVC70)

  IF(MSVC71)
    SET(__install__libs
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
      )
  ENDIF(MSVC71)

  IF(MSVC80)
    # Find the runtime library redistribution directory.
    GET_FILENAME_COMPONENT(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0;InstallDir]" ABSOLUTE)
    FIND_PATH(MSVC80_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC80.CRT/Microsoft.VC80.CRT.manifest
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
      )
    MARK_AS_ADVANCED(MSVC80_REDIST_DIR)
    SET(MSVC80_CRT_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      SET(__install__libs
        "${MSVC80_CRT_DIR}/Microsoft.VC80.CRT.manifest"
        "${MSVC80_CRT_DIR}/msvcm80.dll"
        "${MSVC80_CRT_DIR}/msvcp80.dll"
        "${MSVC80_CRT_DIR}/msvcr80.dll"
        )
    ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
      SET(MSVC80_CRT_DIR
        "${MSVC80_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC80.DebugCRT")
      SET(__install__libs ${__install__libs}
        "${MSVC80_CRT_DIR}/Microsoft.VC80.DebugCRT.manifest"
        "${MSVC80_CRT_DIR}/msvcm80d.dll"
        "${MSVC80_CRT_DIR}/msvcp80d.dll"
        "${MSVC80_CRT_DIR}/msvcr80d.dll"
        )
    ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)
  ENDIF(MSVC80)

  IF(MSVC90)
    # Find the runtime library redistribution directory.
    GET_FILENAME_COMPONENT(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\9.0;InstallDir]" ABSOLUTE)
    GET_FILENAME_COMPONENT(msvc_express_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\9.0;InstallDir]" ABSOLUTE)
    FIND_PATH(MSVC90_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT/Microsoft.VC90.CRT.manifest
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${msvc_express_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
      )
    MARK_AS_ADVANCED(MSVC90_REDIST_DIR)
    SET(MSVC90_CRT_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      SET(__install__libs
        "${MSVC90_CRT_DIR}/Microsoft.VC90.CRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90.dll"
        "${MSVC90_CRT_DIR}/msvcp90.dll"
        "${MSVC90_CRT_DIR}/msvcr90.dll"
        )
    ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
      SET(MSVC90_CRT_DIR
        "${MSVC90_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC90.DebugCRT")
      SET(__install__libs ${__install__libs}
        "${MSVC90_CRT_DIR}/Microsoft.VC90.DebugCRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90d.dll"
        "${MSVC90_CRT_DIR}/msvcp90d.dll"
        "${MSVC90_CRT_DIR}/msvcr90d.dll"
        )
    ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)
  ENDIF(MSVC90)

  IF(MSVC10)
    # Find the runtime library redistribution directory.
    GET_FILENAME_COMPONENT(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;InstallDir]" ABSOLUTE)
    FIND_PATH(MSVC10_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC100.CRT
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
        "$ENV{ProgramFiles}/Microsoft Visual Studio 10.0/VC/redist"
        "$ENV{ProgramFiles(x86)}/Microsoft Visual Studio 10.0/VC/redist"
      )
    MARK_AS_ADVANCED(MSVC10_REDIST_DIR)
    SET(MSVC10_CRT_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.CRT")

    IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      SET(__install__libs
        "${MSVC10_CRT_DIR}/msvcp100.dll"
        "${MSVC10_CRT_DIR}/msvcr100.dll"
        )
    ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
      SET(MSVC10_CRT_DIR
        "${MSVC10_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC100.DebugCRT")
      SET(__install__libs ${__install__libs}
        "${MSVC10_CRT_DIR}/msvcp100d.dll"
        "${MSVC10_CRT_DIR}/msvcr100d.dll"
        )
    ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)
  ENDIF(MSVC10)

  IF(CMAKE_INSTALL_MFC_LIBRARIES)
    IF(MSVC70)
      SET(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc70.dll"
        )
    ENDIF(MSVC70)

    IF(MSVC71)
      SET(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc71.dll"
        )
    ENDIF(MSVC71)

    IF(MSVC80)
      IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
        SET(MSVC80_MFC_DIR
          "${MSVC80_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC80.DebugMFC")
        SET(__install__libs ${__install__libs}
          "${MSVC80_MFC_DIR}/Microsoft.VC80.DebugMFC.manifest"
          "${MSVC80_MFC_DIR}/mfc80d.dll"
          "${MSVC80_MFC_DIR}/mfc80ud.dll"
          "${MSVC80_MFC_DIR}/mfcm80d.dll"
          "${MSVC80_MFC_DIR}/mfcm80ud.dll"
          )
      ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)

      SET(MSVC80_MFC_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.MFC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        SET(__install__libs ${__install__libs}
          "${MSVC80_MFC_DIR}/Microsoft.VC80.MFC.manifest"
          "${MSVC80_MFC_DIR}/mfc80.dll"
          "${MSVC80_MFC_DIR}/mfc80u.dll"
          "${MSVC80_MFC_DIR}/mfcm80.dll"
          "${MSVC80_MFC_DIR}/mfcm80u.dll"
          )
      ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs8 as well as the actuall dll's
      SET(MSVC80_MFCLOC_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.MFCLOC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      SET(__install__libs ${__install__libs}
        "${MSVC80_MFCLOC_DIR}/Microsoft.VC80.MFCLOC.manifest"
        "${MSVC80_MFCLOC_DIR}/mfc80chs.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80cht.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80enu.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80esp.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80deu.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80fra.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80ita.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80jpn.dll"
        "${MSVC80_MFCLOC_DIR}/mfc80kor.dll"
        )
    ENDIF(MSVC80)

    IF(MSVC90)
      IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
        SET(MSVC90_MFC_DIR
          "${MSVC90_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC90.DebugMFC")
        SET(__install__libs ${__install__libs}
          "${MSVC90_MFC_DIR}/Microsoft.VC90.DebugMFC.manifest"
          "${MSVC90_MFC_DIR}/mfc90d.dll"
          "${MSVC90_MFC_DIR}/mfc90ud.dll"
          "${MSVC90_MFC_DIR}/mfcm90d.dll"
          "${MSVC90_MFC_DIR}/mfcm90ud.dll"
          )
      ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)

      SET(MSVC90_MFC_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.MFC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        SET(__install__libs ${__install__libs}
          "${MSVC90_MFC_DIR}/Microsoft.VC90.MFC.manifest"
          "${MSVC90_MFC_DIR}/mfc90.dll"
          "${MSVC90_MFC_DIR}/mfc90u.dll"
          "${MSVC90_MFC_DIR}/mfcm90.dll"
          "${MSVC90_MFC_DIR}/mfcm90u.dll"
          )
      ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs9 as well as the actuall dll's
      SET(MSVC90_MFCLOC_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.MFCLOC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      SET(__install__libs ${__install__libs}
        "${MSVC90_MFCLOC_DIR}/Microsoft.VC90.MFCLOC.manifest"
        "${MSVC90_MFCLOC_DIR}/mfc90chs.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90cht.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90enu.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90esp.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90deu.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90fra.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90ita.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90jpn.dll"
        "${MSVC90_MFCLOC_DIR}/mfc90kor.dll"
        )
    ENDIF(MSVC90)

    IF(MSVC10)
      IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
        SET(MSVC10_MFC_DIR
          "${MSVC10_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC100.DebugMFC")
        SET(__install__libs ${__install__libs}
          "${MSVC10_MFC_DIR}/mfc100d.dll"
          "${MSVC10_MFC_DIR}/mfc100ud.dll"
          "${MSVC10_MFC_DIR}/mfcm100d.dll"
          "${MSVC10_MFC_DIR}/mfcm100ud.dll"
          )
      ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)

      SET(MSVC10_MFC_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.MFC")
      IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        SET(__install__libs ${__install__libs}
          "${MSVC10_MFC_DIR}/mfc100.dll"
          "${MSVC10_MFC_DIR}/mfc100u.dll"
          "${MSVC10_MFC_DIR}/mfcm100.dll"
          "${MSVC10_MFC_DIR}/mfcm100u.dll"
          )
      ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs10 as well as the actuall dll's
      SET(MSVC10_MFCLOC_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.MFCLOC")
      SET(__install__libs ${__install__libs}
        "${MSVC10_MFCLOC_DIR}/mfc100chs.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100cht.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100enu.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100esp.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100deu.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100fra.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100ita.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100jpn.dll"
        "${MSVC10_MFCLOC_DIR}/mfc100kor.dll"
        )
    ENDIF(MSVC10)
  ENDIF(CMAKE_INSTALL_MFC_LIBRARIES)

  FOREACH(lib
      ${__install__libs}
      )
    IF(EXISTS ${lib})
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    ELSE(EXISTS ${lib})
      IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
        MESSAGE(WARNING "system runtime library file does not exist: '${lib}'")
        # This warning indicates an incomplete Visual Studio installation
        # or a bug somewhere above here in this file.
        # If you would like to avoid this warning, fix the real problem, or
        # set CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS before including
        # this file.
      ENDIF()
    ENDIF(EXISTS ${lib})
  ENDFOREACH(lib)
ENDIF(MSVC)

# Include system runtime libraries in the installation if any are
# specified by CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
    IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
      IF(WIN32)
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
      ELSE(WIN32)
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION lib)
      ENDIF(WIN32)
    ENDIF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
    INSTALL(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
  ENDIF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
