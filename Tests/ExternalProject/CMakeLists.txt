cmake_minimum_required(VERSION 2.6)
project(ExternalProjectTest NONE)

include(AddExternalProject)

get_external_project_directories(base_dir build_dir downloads_dir install_dir
  sentinels_dir source_dir tmp_dir)

set(prefix "${install_dir}")


# Local DIR:
#
set(proj TutorialStep5-Local)
add_external_project(${proj}
  DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Tutorial/Step5"
  CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
)


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
  TGZ "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tgz"
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
if(CVS_EXECUTABLE)
  # CVS by date stamp:
  #
  set(proj KWStyle-20081201)
  add_external_project(${proj}
    CVS_REPOSITORY ":pserver:anoncvs@public.kitware.com:/cvsroot/KWStyle"
    CVS_MODULE "KWStyle"
    CVS_TAG "-D\;2008-12-01 01:00:00 UTC"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
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

  # Live CVS / HEAD (no CVS_TAG):
  #
  set(proj TutorialStep1-CVSHEAD)
  add_external_project(${proj}
    CVS_REPOSITORY ":pserver:anonymous:cmake@www.cmake.org:/cvsroot/CMake"
    CVS_MODULE "CMake/Tests/Tutorial/Step1"
    CONFIGURE_ARGS "\"-DCMAKE_INSTALL_PREFIX:PATH=${prefix}\" -G \"${CMAKE_GENERATOR}\" \"${source_dir}/${proj}\""
    INSTALL_COMMAND ""
  )
endif()


# Download SVN:
#
if(Subversion_SVN_EXECUTABLE)
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
set(kwstyleXmlFile "${source_dir}/KWStyle-20081201/Testing/Data/0001-KWStyleConfiguration.kws.xml")
set(header "${install_dir}/include/TutorialConfig.h")


# Do at least a smoke test of a built executable from each
# project's build directory...
#
# BuildTree tests:
#
add_test(TutorialStep5-Local-BuildTreeTest
  "${build_dir}/TutorialStep5-Local/Tutorial" 42)

add_test(TutorialStep1-LocalTAR-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalTAR/Tutorial" 36)

add_test(TutorialStep1-LocalNoDirTAR-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalNoDirTAR/Tutorial" 25)

add_test(TutorialStep1-LocalTGZ-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalTGZ/Tutorial" 16)

add_test(TutorialStep1-LocalNoDirTGZ-BuildTreeTest
  "${build_dir}/TutorialStep1-LocalNoDirTGZ/Tutorial" 9)

if(CVS_EXECUTABLE)
  add_test(KWStyle-20081201-BuildTreeTest
    "${build_dir}/KWStyle-20081201/KWStyle" -xml "${kwstyleXmlFile}" "${header}")

  add_test(kwsys-from-CMake-2-6-2-BuildTreeTest
    "${build_dir}/kwsys-from-CMake-2-6-2/kwsysTestProcess" 1)

  add_test(TutorialStep1-CVSHEAD-BuildTreeTest
    "${build_dir}/TutorialStep1-LocalNoDirTGZ/Tutorial" 4)
endif()

if(Subversion_SVN_EXECUTABLE)
  add_test(gdcm-md5-20081204-BuildTreeTest
    "${build_dir}/gdcm-md5-20081204/md5main" --version)

  add_test(gdcm-md5-r4824-BuildTreeTest
    "${build_dir}/gdcm-md5-r4824/md5main" --version)

  add_test(gdcm-md5-SVNtrunk-BuildTreeTest
    "${build_dir}/gdcm-md5-SVNtrunk/md5main" --version)
endif()


# InstallTree tests:
#
add_test(TutorialStep5-InstallTreeTest
  "${install_dir}/bin/Tutorial" 49)

if(CVS_EXECUTABLE)
  add_test(KWStyle-InstallTreeTest
    "${install_dir}/bin/KWStyle" -xml "${kwstyleXmlFile}" "${header}")
endif()

if(Subversion_SVN_EXECUTABLE)
  add_test(gdcm-md5-InstallTreeTest
    "${install_dir}/bin/md5main" --version)
endif()
