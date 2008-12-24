cmake_minimum_required(VERSION 2.6)
project(ExternalProjectTest NONE)

include(AddExternalProject)

get_external_project_directories(base_dir build_dir downloads_dir install_dir
  sentinels_dir source_dir tmp_dir)

set(prefix "${install_dir}")


# Use a "TryCheckout" technique on small subtrees of certain projects
# to see if cvs checkout and svn checkout may be used on this machine
# without problems. If so, we can test the projects that use those
# download techniques. If not, we skip them on this machine...
#
include("${CMAKE_CURRENT_SOURCE_DIR}/TryCheckout.cmake")

if(NOT DEFINED can_build_kwstyle)
  if(WATCOM)
    set(can_build_kwstyle 0)
  else()
    set(can_build_kwstyle 1)
  endif()
endif()

if(NOT DEFINED can_build_tutorial_step5)
  set(can_build_tutorial_step5 1)

  # Tutorial Step5 cannot build correctly using Visual Studio 6
  # on Windows 98 if the path of its build tree exceeds 72
  # characters in length... So don't attempt to build it
  # in a long path on Win98:
  #
  if(CMAKE_SYSTEM STREQUAL "Windows-4.10")
    string(LENGTH "${build_dir}/TutorialStep5-Local" n)
    if(n GREATER 72)
      set(can_build_tutorial_step5 0)
    endif()
  endif()
endif()

if(NOT DEFINED can_use_cvs)
  try_cvs_checkout(
    ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    "CMake/Tests/Tutorial/Step1"
    "${CMAKE_CURRENT_BINARY_DIR}/TryCheckout/TutorialStep1"
    can_use_cvs
    )
  set(can_use_cvs ${can_use_cvs} CACHE STRING "Was try_cvs_checkout successful?")
endif()

if(NOT DEFINED can_use_svn)
  try_svn_checkout(
    "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    "${CMAKE_CURRENT_BINARY_DIR}/TryCheckout/gdcmmd5"
    can_use_svn
    )
  set(can_use_svn ${can_use_svn} CACHE STRING "Was try_svn_checkout successful?")
endif()

message(STATUS "can_build_kwstyle='${can_build_kwstyle}'")
message(STATUS "can_build_tutorial_step5='${can_build_tutorial_step5}'")
message(STATUS "can_use_cvs='${can_use_cvs}'")
message(STATUS "can_use_svn='${can_use_svn}'")


# Empty projects that test all the known add_external_project argument key words:
#
set(proj MinimalNoOpProject)
add_external_project(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)

set(proj EmptyNoOpProject)
add_external_project(${proj}
  BUILD_ARGS ""
  BUILD_COMMAND ""
  CONFIGURE_ARGS ""
  CONFIGURE_COMMAND ""
  CONFIGURE_DIR ""
  CVS_REPOSITORY ""
  CVS_MODULE ""
  CVS_TAG ""
  DEPENDS "MinimalNoOpProject"
  DIR ""
  DOWNLOAD_ARGS ""
  DOWNLOAD_COMMAND ""
  INSTALL_ARGS ""
  INSTALL_COMMAND ""
  SVN_REPOSITORY ""
  SVN_TAG ""
  TAR ""
  TAR_URL ""
  TGZ ""
  TGZ_URL ""
  UPDATE_ARGS ""
  UPDATE_COMMAND ""
)


# Local DIR:
#
if(can_build_tutorial_step5)
  set(proj TutorialStep5-Local)
  add_external_project(${proj}
    DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Tutorial/Step5"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  )
endif()


# Local TAR:
#
set(proj TutorialStep1-LocalTAR)
add_external_project(${proj}
  TAR "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tar"
  CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  INSTALL_COMMAND ""
)

set(proj TutorialStep1-LocalNoDirTAR)
add_external_project(${proj}
  TAR "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tar"
  CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  INSTALL_COMMAND ""
)


# Local TGZ:
#
set(proj TutorialStep1-LocalTGZ)
add_external_project(${proj}
  TGZ "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tgz"
  CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  INSTALL_COMMAND ""
)

set(proj TutorialStep1-LocalNoDirTGZ)
add_external_project(${proj}
  TGZ "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tgz"
  CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  INSTALL_COMMAND ""
)


# Download TAR:
#
# TODO: Add a remote .tar file


# Download TGZ:
#
# TODO: Add a remote .tgz file


# Download CVS:
#
if(can_use_cvs)
  # CVS by date stamp:
  #
  set(proj TutorialStep1-20081201)
  add_external_project(${proj}
    CVS_REPOSITORY ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    CVS_MODULE "CMake/Tests/Tutorial/Step1"
    CVS_TAG "-D\;2008-12-01 01:00:00 UTC"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
    INSTALL_COMMAND ""
  )

  # CVS by tag:
  #
  set(proj kwsys-from-CMake-2-6-2)
  add_external_project(${proj}
    CVS_REPOSITORY ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    CVS_MODULE "CMake/Source/kwsys"
    CVS_TAG "-r\;CMake-2-6-2"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
    INSTALL_COMMAND ""
  )

  if(can_build_kwstyle)
    # Live CVS / HEAD (no CVS_TAG):
    #
    set(proj KWStyle-CVSHEAD)
    add_external_project(${proj}
      CVS_REPOSITORY ":pserver:anoncvs@public.kitware.com:/cvsroot/KWStyle"
      CVS_MODULE "KWStyle"
      CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
      DEPENDS "EmptyNoOpProject"
      DEPENDS "TutorialStep1-LocalTAR"
      DEPENDS "TutorialStep1-LocalNoDirTAR"
      DEPENDS "TutorialStep1-LocalTGZ"
      DEPENDS "TutorialStep1-LocalNoDirTGZ"
      DEPENDS "TutorialStep1-20081201"
      DEPENDS "kwsys-from-CMake-2-6-2"
    )
  endif()
endif()


# Download SVN:
#
if(can_use_svn)
  # SVN by date stamp:
  #
  set(proj gdcm-md5-20081204)
  add_external_project(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    SVN_TAG "-r\;{2008-12-04 01:00:00 +0000}"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
  )

  # SVN by revision number:
  #
  set(proj gdcm-md5-r4824)
  add_external_project(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    SVN_TAG "-r\;4824"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
    INSTALL_COMMAND ""
  )

  # Live SVN / trunk (no SVN_TAG):
  #
  set(proj gdcm-md5-SVNtrunk)
  add_external_project(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
    INSTALL_COMMAND ""
  )
endif()


# Test the testable built/installed products:
#
enable_testing()


# Use these as input to the KWStyle tests:
#
set(kwstyleXmlFile "${source_dir}/KWStyle-CVSHEAD/Testing/Data/0001-KWStyleConfiguration.kws.xml")
set(header "${install_dir}/include/TutorialConfig.h")


# Do at least a smoke test of a built executable from each
# project's build directory...
#
# BuildTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-Local-BuildTreeTest
    "${build_dir}/TutorialStep5-Local/Tutorial" 42)
endif()

add_test(TutorialStep1-LocalTAR-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalTAR/Tutorial" 36)

add_test(TutorialStep1-LocalNoDirTAR-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalNoDirTAR/Tutorial" 25)

add_test(TutorialStep1-LocalTGZ-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalTGZ/Tutorial" 16)

add_test(TutorialStep1-LocalNoDirTGZ-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalNoDirTGZ/Tutorial" 9)

if(can_use_cvs)
  add_test(TutorialStep1-20081201-BuildTreeTest
    "${build_dir}/TutorialStep1-20081201/Tutorial" 4)

  add_test(kwsys-from-CMake-2-6-2-BuildTreeTest
    "${build_dir}/kwsys-from-CMake-2-6-2/kwsysTestProcess" 1)

  if(can_build_kwstyle)
    add_test(KWStyle-CVSHEAD-BuildTreeTest
      "${build_dir}/KWStyle-CVSHEAD/KWStyle" -xml "${kwstyleXmlFile}" "${header}")
  endif()
endif()

if(can_use_svn)
  add_test(gdcm-md5-20081204-BuildTreeTest
    "${build_dir}/gdcm-md5-20081204/md5main" --version)

  add_test(gdcm-md5-r4824-BuildTreeTest
    "${build_dir}/gdcm-md5-r4824/md5main" --version)

  add_test(gdcm-md5-SVNtrunk-BuildTreeTest
    "${build_dir}/gdcm-md5-SVNtrunk/md5main" --version)
endif()


# InstallTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-InstallTreeTest
    "${install_dir}/bin/Tutorial" 49)
endif()

if(can_use_cvs)
  if(can_build_kwstyle)
    add_test(KWStyle-InstallTreeTest
      "${install_dir}/bin/KWStyle" -xml "${kwstyleXmlFile}" "${header}")
  endif()
endif()

if(can_use_svn)
  add_test(gdcm-md5-InstallTreeTest
    "${install_dir}/bin/md5main" --version)
endif()
