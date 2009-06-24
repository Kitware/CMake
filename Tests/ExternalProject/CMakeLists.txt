cmake_minimum_required(VERSION 2.6)
project(ExternalProjectTest NONE)

include(ExternalProject)

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
set(binary_base "${base}/Build")
set_property(DIRECTORY PROPERTY EP_BASE ${base})

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
    string(LENGTH "${binary_base}/TutorialStep5-Local" n)
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


# Empty projects that test all the known ep_add argument key words:
#
set(proj MinimalNoOpProject)
ep_add(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)

set(proj EmptyNoOpProject)
ep_add(${proj}
  BUILD_COMMAND ""
  CMAKE_ARGS ""
  CONFIGURE_COMMAND ""
  CVS_REPOSITORY ""
  CVS_MODULE ""
  CVS_TAG ""
  DEPENDS "MinimalNoOpProject"
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  PATCH_COMMAND ""
  SVN_REPOSITORY ""
  SVN_TAG ""
  URL ""
  UPDATE_COMMAND ""
)


# Local DIR:
#
if(can_build_tutorial_step5)
  set(proj TutorialStep5-Local)
  ep_add(${proj}
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../Tutorial/Step5"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
  )
  ep_get(${proj} install_dir)
  set(TutorialStep5_install_dir ${install_dir})
endif()


# Local TAR:
#
set(proj TutorialStep1-LocalTAR)
ep_add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tar"
  LIST_SEPARATOR ::
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Step1Patch.cmake
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
             -DTEST_LIST:STRING=A::B::C
  INSTALL_COMMAND ""
)

set(proj TutorialStep1-LocalNoDirTAR)
ep_add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tar"
  LIST_SEPARATOR @@
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
             -DTEST_LIST:STRING=1@@2@@3
  INSTALL_COMMAND ""
)
ep_add_step(${proj} mypatch
  COMMAND ${CMAKE_COMMAND} -E echo "This is a custom external project step."
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Step1Patch.cmake
  WORKING_DIRECTORY <SOURCE_DIR>
  DEPENDEES download
  DEPENDERS configure
  )


# Local TGZ:
#
set(proj TutorialStep1-LocalTGZ)
ep_add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tgz"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
  INSTALL_COMMAND ""
)

set(proj TutorialStep1-LocalNoDirTGZ)
ep_add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tgz"
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
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
  ep_add(${proj}
    CVS_REPOSITORY ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    CVS_MODULE "CMake/Tests/Tutorial/Step1"
    CVS_TAG "-D2008-12-01 01:00:00 UTC"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
  )

  # CVS by tag:
  #
  set(proj kwsys-from-CMake-2-6-2)
  ep_add(${proj}
    CVS_REPOSITORY ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    CVS_MODULE "CMake/Source/kwsys"
    CVS_TAG -rCMake-2-6-2
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
  )

  if(can_build_kwstyle)
    # Live CVS / HEAD (no CVS_TAG):
    #
    set(proj KWStyle-CVSHEAD)
    ep_add(${proj}
      CVS_REPOSITORY ":pserver:anoncvs@public.kitware.com:/cvsroot/KWStyle"
      CVS_MODULE "KWStyle"
      CMAKE_GENERATOR "${CMAKE_GENERATOR}"
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      DEPENDS "EmptyNoOpProject"
      DEPENDS "TutorialStep1-LocalTAR"
      DEPENDS "TutorialStep1-LocalNoDirTAR"
      DEPENDS "TutorialStep1-LocalTGZ"
      DEPENDS "TutorialStep1-LocalNoDirTGZ"
      DEPENDS "TutorialStep1-20081201"
      DEPENDS "kwsys-from-CMake-2-6-2"
    )
    ep_get(${proj} source_dir install_dir)
    set(kwstyle_source_dir ${source_dir})
    set(kwstyle_install_dir ${install_dir})
  endif()
endif()


# Download SVN:
#
if(can_use_svn)
  # SVN by date stamp:
  #
  set(proj gdcm-md5-20081204)
  ep_add(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    SVN_TAG "-r{2008-12-04 01:00:00 +0000}"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  )
  ep_get(${proj} install_dir)
  set(gdcm_install_dir ${install_dir})

  # SVN by revision number:
  #
  set(proj gdcm-md5-r4824)
  ep_add(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    SVN_TAG "-r4824"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
  )

  # Live SVN / trunk (no SVN_TAG):
  #
  set(proj gdcm-md5-SVNtrunk)
  ep_add(${proj}
    SVN_REPOSITORY "http://gdcm.svn.sourceforge.net/svnroot/gdcm/trunk/Utilities/gdcmmd5"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
  )
endif()


# Test the testable built/installed products:
#
enable_testing()


# Use these as input to the KWStyle tests:
#
set(kwstyleXmlFile "${kwstyle_source_dir}/Testing/Data/0001-KWStyleConfiguration.kws.xml")
set(header "${TutorialStep5_install_dir}/include/TutorialConfig.h")


# Do at least a smoke test of a built executable from each
# project's build directory...
#
# BuildTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-Local-BuildTreeTest
    "${binary_base}/TutorialStep5-Local/Tutorial" 42)
endif()

add_test(TutorialStep1-LocalTAR-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalTAR/EP-Tutorial" 36)

add_test(TutorialStep1-LocalNoDirTAR-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalNoDirTAR/EP-Tutorial" 25)

add_test(TutorialStep1-LocalTGZ-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalTGZ/Tutorial" 16)

add_test(TutorialStep1-LocalNoDirTGZ-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalNoDirTGZ/Tutorial" 9)

if(can_use_cvs)
  add_test(TutorialStep1-20081201-BuildTreeTest
    "${binary_base}/TutorialStep1-20081201/Tutorial" 4)

  add_test(kwsys-from-CMake-2-6-2-BuildTreeTest
    "${binary_base}/kwsys-from-CMake-2-6-2/kwsysTestProcess" 1)

  if(can_build_kwstyle)
    add_test(KWStyle-CVSHEAD-BuildTreeTest
      "${binary_base}/KWStyle-CVSHEAD/KWStyle" -xml "${kwstyleXmlFile}" "${header}")
  endif()
endif()

if(can_use_svn)
  add_test(gdcm-md5-20081204-BuildTreeTest
    "${binary_base}/gdcm-md5-20081204/md5main" --version)

  add_test(gdcm-md5-r4824-BuildTreeTest
    "${binary_base}/gdcm-md5-r4824/md5main" --version)

  add_test(gdcm-md5-SVNtrunk-BuildTreeTest
    "${binary_base}/gdcm-md5-SVNtrunk/md5main" --version)
endif()


# InstallTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-InstallTreeTest
    "${TutorialStep5_install_dir}/bin/Tutorial" 49)
endif()

if(can_use_cvs)
  if(can_build_kwstyle)
    add_test(KWStyle-InstallTreeTest
      "${kwstyle_install_dir}/bin/KWStyle" -xml "${kwstyleXmlFile}" "${header}")
  endif()
endif()

if(can_use_svn)
  add_test(gdcm-md5-InstallTreeTest
    "${gdcm_install_dir}/bin/md5main" --version)
endif()
