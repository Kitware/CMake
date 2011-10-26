# By including this file, all library files listed in the variable
# CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS will be installed with
# install(PROGRAMS ...) into bin for WIN32 and lib
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

if(MSVC)
  file(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)

  if(CMAKE_CL_64)
    if(MSVC_VERSION GREATER 1599)
      # VS 10 and later:
      set(CMAKE_MSVC_ARCH x64)
    else()
      # VS 9 and earlier:
      set(CMAKE_MSVC_ARCH amd64)
    endif()
  else(CMAKE_CL_64)
    set(CMAKE_MSVC_ARCH x86)
  endif(CMAKE_CL_64)

  get_filename_component(devenv_dir "${CMAKE_MAKE_PROGRAM}" PATH)
  get_filename_component(base_dir "${devenv_dir}/../.." ABSOLUTE)

  if(MSVC70)
    set(__install__libs
      "${SYSTEMROOT}/system32/msvcp70.dll"
      "${SYSTEMROOT}/system32/msvcr70.dll"
      )
  endif(MSVC70)

  if(MSVC71)
    set(__install__libs
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
      )
  endif(MSVC71)

  if(MSVC80)
    # Find the runtime library redistribution directory.
    get_filename_component(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0;InstallDir]" ABSOLUTE)
    find_path(MSVC80_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC80.CRT/Microsoft.VC80.CRT.manifest
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
      )
    mark_as_advanced(MSVC80_REDIST_DIR)
    set(MSVC80_CRT_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      set(__install__libs
        "${MSVC80_CRT_DIR}/Microsoft.VC80.CRT.manifest"
        "${MSVC80_CRT_DIR}/msvcm80.dll"
        "${MSVC80_CRT_DIR}/msvcp80.dll"
        "${MSVC80_CRT_DIR}/msvcr80.dll"
        )
    endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    if(CMAKE_INSTALL_DEBUG_LIBRARIES)
      set(MSVC80_CRT_DIR
        "${MSVC80_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC80.DebugCRT")
      set(__install__libs ${__install__libs}
        "${MSVC80_CRT_DIR}/Microsoft.VC80.DebugCRT.manifest"
        "${MSVC80_CRT_DIR}/msvcm80d.dll"
        "${MSVC80_CRT_DIR}/msvcp80d.dll"
        "${MSVC80_CRT_DIR}/msvcr80d.dll"
        )
    endif(CMAKE_INSTALL_DEBUG_LIBRARIES)
  endif(MSVC80)

  if(MSVC90)
    # Find the runtime library redistribution directory.
    get_filename_component(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\9.0;InstallDir]" ABSOLUTE)
    get_filename_component(msvc_express_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\9.0;InstallDir]" ABSOLUTE)
    find_path(MSVC90_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT/Microsoft.VC90.CRT.manifest
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${msvc_express_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
      )
    mark_as_advanced(MSVC90_REDIST_DIR)
    set(MSVC90_CRT_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      set(__install__libs
        "${MSVC90_CRT_DIR}/Microsoft.VC90.CRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90.dll"
        "${MSVC90_CRT_DIR}/msvcp90.dll"
        "${MSVC90_CRT_DIR}/msvcr90.dll"
        )
    endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    if(CMAKE_INSTALL_DEBUG_LIBRARIES)
      set(MSVC90_CRT_DIR
        "${MSVC90_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC90.DebugCRT")
      set(__install__libs ${__install__libs}
        "${MSVC90_CRT_DIR}/Microsoft.VC90.DebugCRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90d.dll"
        "${MSVC90_CRT_DIR}/msvcp90d.dll"
        "${MSVC90_CRT_DIR}/msvcr90d.dll"
        )
    endif(CMAKE_INSTALL_DEBUG_LIBRARIES)
  endif(MSVC90)

  if(MSVC10)
    # Find the runtime library redistribution directory.
    get_filename_component(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;InstallDir]" ABSOLUTE)
    find_path(MSVC10_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC100.CRT
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
        "$ENV{ProgramFiles}/Microsoft Visual Studio 10.0/VC/redist"
        "$ENV{ProgramFiles(x86)}/Microsoft Visual Studio 10.0/VC/redist"
      )
    mark_as_advanced(MSVC10_REDIST_DIR)
    set(MSVC10_CRT_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.CRT")

    if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      set(__install__libs
        "${MSVC10_CRT_DIR}/msvcp100.dll"
        "${MSVC10_CRT_DIR}/msvcr100.dll"
        )
    endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    if(CMAKE_INSTALL_DEBUG_LIBRARIES)
      set(MSVC10_CRT_DIR
        "${MSVC10_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC100.DebugCRT")
      set(__install__libs ${__install__libs}
        "${MSVC10_CRT_DIR}/msvcp100d.dll"
        "${MSVC10_CRT_DIR}/msvcr100d.dll"
        )
    endif(CMAKE_INSTALL_DEBUG_LIBRARIES)
  endif(MSVC10)

  if(CMAKE_INSTALL_MFC_LIBRARIES)
    if(MSVC70)
      set(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc70.dll"
        )
    endif(MSVC70)

    if(MSVC71)
      set(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc71.dll"
        )
    endif(MSVC71)

    if(MSVC80)
      if(CMAKE_INSTALL_DEBUG_LIBRARIES)
        set(MSVC80_MFC_DIR
          "${MSVC80_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC80.DebugMFC")
        set(__install__libs ${__install__libs}
          "${MSVC80_MFC_DIR}/Microsoft.VC80.DebugMFC.manifest"
          "${MSVC80_MFC_DIR}/mfc80d.dll"
          "${MSVC80_MFC_DIR}/mfc80ud.dll"
          "${MSVC80_MFC_DIR}/mfcm80d.dll"
          "${MSVC80_MFC_DIR}/mfcm80ud.dll"
          )
      endif(CMAKE_INSTALL_DEBUG_LIBRARIES)

      set(MSVC80_MFC_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.MFC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        set(__install__libs ${__install__libs}
          "${MSVC80_MFC_DIR}/Microsoft.VC80.MFC.manifest"
          "${MSVC80_MFC_DIR}/mfc80.dll"
          "${MSVC80_MFC_DIR}/mfc80u.dll"
          "${MSVC80_MFC_DIR}/mfcm80.dll"
          "${MSVC80_MFC_DIR}/mfcm80u.dll"
          )
      endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs8 as well as the actuall dll's
      set(MSVC80_MFCLOC_DIR "${MSVC80_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC80.MFCLOC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      set(__install__libs ${__install__libs}
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
    endif(MSVC80)

    if(MSVC90)
      if(CMAKE_INSTALL_DEBUG_LIBRARIES)
        set(MSVC90_MFC_DIR
          "${MSVC90_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC90.DebugMFC")
        set(__install__libs ${__install__libs}
          "${MSVC90_MFC_DIR}/Microsoft.VC90.DebugMFC.manifest"
          "${MSVC90_MFC_DIR}/mfc90d.dll"
          "${MSVC90_MFC_DIR}/mfc90ud.dll"
          "${MSVC90_MFC_DIR}/mfcm90d.dll"
          "${MSVC90_MFC_DIR}/mfcm90ud.dll"
          )
      endif(CMAKE_INSTALL_DEBUG_LIBRARIES)

      set(MSVC90_MFC_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.MFC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        set(__install__libs ${__install__libs}
          "${MSVC90_MFC_DIR}/Microsoft.VC90.MFC.manifest"
          "${MSVC90_MFC_DIR}/mfc90.dll"
          "${MSVC90_MFC_DIR}/mfc90u.dll"
          "${MSVC90_MFC_DIR}/mfcm90.dll"
          "${MSVC90_MFC_DIR}/mfcm90u.dll"
          )
      endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs9 as well as the actuall dll's
      set(MSVC90_MFCLOC_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.MFCLOC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      set(__install__libs ${__install__libs}
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
    endif(MSVC90)

    if(MSVC10)
      if(CMAKE_INSTALL_DEBUG_LIBRARIES)
        set(MSVC10_MFC_DIR
          "${MSVC10_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC100.DebugMFC")
        set(__install__libs ${__install__libs}
          "${MSVC10_MFC_DIR}/mfc100d.dll"
          "${MSVC10_MFC_DIR}/mfc100ud.dll"
          "${MSVC10_MFC_DIR}/mfcm100d.dll"
          "${MSVC10_MFC_DIR}/mfcm100ud.dll"
          )
      endif(CMAKE_INSTALL_DEBUG_LIBRARIES)

      set(MSVC10_MFC_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.MFC")
      if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
        set(__install__libs ${__install__libs}
          "${MSVC10_MFC_DIR}/mfc100.dll"
          "${MSVC10_MFC_DIR}/mfc100u.dll"
          "${MSVC10_MFC_DIR}/mfcm100.dll"
          "${MSVC10_MFC_DIR}/mfcm100u.dll"
          )
      endif(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

      # include the language dll's for vs10 as well as the actuall dll's
      set(MSVC10_MFCLOC_DIR "${MSVC10_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC100.MFCLOC")
      set(__install__libs ${__install__libs}
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
    endif(MSVC10)
  endif(CMAKE_INSTALL_MFC_LIBRARIES)

  foreach(lib
      ${__install__libs}
      )
    if(EXISTS ${lib})
      set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    else(EXISTS ${lib})
      if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
        message(WARNING "system runtime library file does not exist: '${lib}'")
        # This warning indicates an incomplete Visual Studio installation
        # or a bug somewhere above here in this file.
        # If you would like to avoid this warning, fix the real problem, or
        # set CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS before including
        # this file.
      endif()
    endif(EXISTS ${lib})
  endforeach(lib)
endif(MSVC)

if(WATCOM)
  get_filename_component( CompilerPath ${CMAKE_C_COMPILER} PATH )
  if(WATCOM17)
     set( __install__libs ${CompilerPath}/clbr17.dll
       ${CompilerPath}/mt7r17.dll ${CompilerPath}/plbr17.dll )
  endif()
  if(WATCOM18)
     set( __install__libs ${CompilerPath}/clbr18.dll
       ${CompilerPath}/mt7r18.dll ${CompilerPath}/plbr18.dll )
  endif()
  if(WATCOM19)
     set( __install__libs ${CompilerPath}/clbr19.dll
       ${CompilerPath}/mt7r19.dll ${CompilerPath}/plbr19.dll )
  endif()
  foreach(lib
      ${__install__libs}
      )
    if(EXISTS ${lib})
      set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    else()
      if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
        message(WARNING "system runtime library file does not exist: '${lib}'")
        # This warning indicates an incomplete Watcom installation
        # or a bug somewhere above here in this file.
        # If you would like to avoid this warning, fix the real problem, or
        # set CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS before including
        # this file.
      endif()
    endif()
  endforeach()
endif()


# Include system runtime libraries in the installation if any are
# specified by CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.
if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
    if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
      if(WIN32)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
      else(WIN32)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION lib)
      endif(WIN32)
    endif(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
  endif(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
endif(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
