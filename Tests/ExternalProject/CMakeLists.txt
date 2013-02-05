cmake_minimum_required(VERSION 2.8)
project(ExternalProjectTest NONE)

include(ExternalProject)

find_package(CVS)
find_package(Subversion)
find_package(Git)
find_package(Hg)

option(ExternalProjectTest_USE_FOLDERS "Enable folder grouping in IDEs." ON)
if(ExternalProjectTest_USE_FOLDERS)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
  set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
endif()

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER
  "CMakePredefinedTargets-in-ExternalProjectTest")

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
set(binary_base "${base}/Build")
set_property(DIRECTORY PROPERTY EP_BASE ${base})
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

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

  # The ExternalProject builds of Tutorial Step5 cannot be built
  # correctly 2nd and later times in an in-source build...
  # (because the CMakeCache.txt from the real in-source build of
  # the Tests/Tutorial/Step5 directory gets copied when we do
  # the "source directory copy" step... but it still refers to
  # its original path which yields a configure error.) So:
  #
  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    set(can_build_tutorial_step5 0)
  endif()
endif()

add_custom_target(NonExternalProjectTarget
  COMMAND ${CMAKE_COMMAND} -E echo NonExternalProjectTarget)

# Empty projects that test all the known ExternalProject_Add argument key words:
#
set(proj AAA-TestAlphabetization)
ExternalProject_Add(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)

set(proj ZZZ-TestAlphabetization)
ExternalProject_Add(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)

set(proj TargetNameSameAsFolder)
ExternalProject_Add(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "${proj}")

set(proj MinimalNoOpProject)
ExternalProject_Add(${proj}
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
)

set(proj EmptyNoOpProject)
ExternalProject_Add(${proj}
  BUILD_COMMAND ""
  CMAKE_ARGS ""
  CONFIGURE_COMMAND ""
  CVS_REPOSITORY ""
  CVS_MODULE ""
  CVS_TAG ""
  DEPENDS "MinimalNoOpProject" NonExternalProjectTarget
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  PATCH_COMMAND ""
  STEP_TARGETS install update
  SVN_REPOSITORY ""
  SVN_REVISION ""
  SVN_USERNAME ""
  SVN_PASSWORD ""
  SVN_TRUST_CERT 1
  TEST_COMMAND ""
  TIMEOUT ""
  URL ""
  URL_MD5 ""
  UPDATE_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "")


# Local DIR:
#
if(can_build_tutorial_step5)
  set(proj TutorialStep5-Local)
  ExternalProject_Add(${proj}
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../Tutorial/Step5"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    CMAKE_ARGS -G ${CMAKE_GENERATOR} <SOURCE_DIR>
    TEST_BEFORE_INSTALL 1
    LOG_INSTALL 1
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "Local")
  ExternalProject_Get_Property(${proj} install_dir)
  set(TutorialStep5_install_dir ${install_dir})

  set(proj TutorialStep5-Local-TestAfterInstall)
  ExternalProject_Add(${proj}
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../Tutorial/Step5"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
    TEST_AFTER_INSTALL 1
    LOG_TEST 1
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "Local")
endif()


# Local TAR:
#
set(proj TutorialStep1-LocalTAR)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tar"
  URL_MD5 a87c5b47c0201c09ddfe1d5738fdb1e3
  LIST_SEPARATOR ::
  PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Step1Patch.cmake
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
             -DTEST_LIST:STRING=A::B::C
  INSTALL_COMMAND ""
  LOG_CONFIGURE 1
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/TAR")

set(proj TutorialStep1-LocalNoDirTAR)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tar"
  URL_MD5 d09e3d370c5c908fa035c30939ee438e
  LIST_SEPARATOR @@
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
             -DTEST_LIST:STRING=1@@2@@3
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/TAR")
ExternalProject_Add_Step(${proj} mypatch
  COMMAND ${CMAKE_COMMAND} -E echo "This is a custom external project step."
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Step1Patch.cmake
  WORKING_DIRECTORY <SOURCE_DIR>
  DEPENDEES download
  DEPENDERS configure
  )


# Local TGZ:
#
set(proj TutorialStep1-LocalTGZ)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tgz"
  URL_MD5 38c648e817339c356f6be00eeed79bd0
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR} <SOURCE_DIR>
  INSTALL_COMMAND ""
  LOG_BUILD 1
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/TGZ")

set(proj TutorialStep1-LocalNoDirTGZ)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tgz"
  URL_HASH SHA256=496229e2a5ed620a37c385ad9406004a18026beab8b55dd2c4565d4b7f1d5383
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/TGZ")


# Local BZ2:
#
# (The bz2 tests are here just to verify that the bz2 decompression is executed
#  during a test suite run... The configure and build commands are set to
#  nothing to make the test quicker. To make this more complete, I should add
#  a diff between this and the TGZ source tree since that one does build...)
#
set(proj TutorialStep1-LocalBZ2)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.tar.bz2"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/BZ2")

set(proj TutorialStep1-LocalNoDirBZ2)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.tar.bz2"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/BZ2")


# Local ZIP:
#
# (The zip tests are here just to verify that the zip decompression is executed
#  during a test suite run... The configure and build commands are set to
#  nothing to make the test quicker. To make this more complete, I should add
#  a diff between this and the TGZ source tree since that one does build...)
#
set(proj TutorialStep1-LocalZIP)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1.zip"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/ZIP")

set(proj TutorialStep1-LocalNoDirZIP)
ExternalProject_Add(${proj}
  URL "${CMAKE_CURRENT_SOURCE_DIR}/Step1NoDir.zip"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set_property(TARGET ${proj} PROPERTY FOLDER "Local/ZIP")


# CVS-based tests:
#
set(do_cvs_tests 0)

if(CVS_EXECUTABLE)
  set(do_cvs_tests 1)
endif()

if(do_cvs_tests AND NOT UNIX)
  if("${CVS_EXECUTABLE}" MATCHES "cygwin")
    message(STATUS "No ExternalProject cvs tests with cygwin cvs.exe outside cygwin!")
    set(do_cvs_tests 0)
  endif()
endif()

if(do_cvs_tests)
  # Unzip/untar the CVS repository in our source folder so that other
  # projects below may use it to test CVS args of ExternalProject_Add
  #
  set(proj SetupLocalCVSRepository)
  set(local_cvs_repo "${CMAKE_CURRENT_BINARY_DIR}/LocalRepositories/CVS")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${local_cvs_repo}
    URL ${CMAKE_CURRENT_SOURCE_DIR}/cvsrepo.tgz
    URL_MD5 55fc85825ffdd9ed2597123c68b79f7e
    BUILD_COMMAND ""
    CONFIGURE_COMMAND "${CVS_EXECUTABLE}" --version
    INSTALL_COMMAND ""
  )
  set_property(TARGET ${proj}
    PROPERTY FOLDER "SetupRepos/Local/Deeply/Nested/For/Testing")

  # The MSYS cvs tool interprets "c:/" as a "machine:" name for SSH.
  # Detect the MSYS cvs and convert the repo path to an MSYS path.
  if(WIN32)
    if(EXISTS "${CVS_EXECUTABLE}")
      file(STRINGS "${CVS_EXECUTABLE}" cvs_is_msys LIMIT_COUNT 1 REGEX "[Mm][Ss][Yy][Ss]")
      if(cvs_is_msys)
        message(STATUS "'${CVS_EXECUTABLE}' is from MSYS (contains '${cvs_is_msys}')")
        string(REGEX REPLACE "^([A-Za-z]):" "/\\1" local_cvs_repo "${local_cvs_repo}")
      endif()
    endif()
  endif()

  # CVS by date stamp:
  #
  set(proj TutorialStep1-CVS-20090626)
  ExternalProject_Add(${proj}
    CVS_REPOSITORY "${local_cvs_repo}"
    CVS_MODULE "TutorialStep1"
    CVS_TAG "-D2009-06-26 16:50:00 UTC"
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalCVSRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "CVS")

  # CVS by tag:
  #
  set(proj TutorialStep1-CVS-testtag1)
  ExternalProject_Add(${proj}
    CVS_REPOSITORY "${local_cvs_repo}"
    CVS_MODULE "TutorialStep1"
    CVS_TAG -rtesttag1
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalCVSRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "CVS")

  # Live CVS / HEAD (no CVS_TAG):
  #
  set(proj TutorialStep1-CVS-HEAD)
  ExternalProject_Add(${proj}
    CVS_REPOSITORY "${local_cvs_repo}"
    CVS_MODULE "TutorialStep1"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalCVSRepository"
    DEPENDS "EmptyNoOpProject"
    DEPENDS "TutorialStep1-LocalTAR"
    DEPENDS "TutorialStep1-LocalNoDirTAR"
    DEPENDS "TutorialStep1-LocalTGZ"
    DEPENDS "TutorialStep1-LocalNoDirTGZ"
    DEPENDS "TutorialStep1-CVS-20090626"
    DEPENDS "TutorialStep1-CVS-testtag1"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "CVS")
endif()


# SVN-based tests:
#
set(do_svn_tests 0)

if(Subversion_SVN_EXECUTABLE)
  set(do_svn_tests 1)
endif()

# Only do svn tests with svn >= version 1.2
#
if(do_svn_tests)
  if(Subversion_VERSION_SVN VERSION_LESS 1.2)
    message(STATUS "No ExternalProject svn tests with svn client less than version 1.2")
    set(do_svn_tests 0)
  endif()
endif()

# Only do svn tests in cygwin/cygwin or not-cygwin/not-cygwin arrangements:
#
if(do_svn_tests)
  if(CMAKE_CURRENT_BINARY_DIR MATCHES "cygdrive/" AND
     NOT "${Subversion_SVN_EXECUTABLE}" MATCHES "cygwin")
    message(STATUS "No ExternalProject svn tests with non-cygwin svn client in a /cygdrive based build")
    set(do_svn_tests 0)
  endif()
endif()

if(do_svn_tests)
  # Unzip/untar the SVN repository in our source folder so that other
  # projects below may use it to test SVN args of ExternalProject_Add
  #
  set(proj SetupLocalSVNRepository)
  set(local_svn_repo "${CMAKE_CURRENT_BINARY_DIR}/LocalRepositories/SVN")
  set(local_svn_repo_url "file:///${local_svn_repo}/TutorialStep1")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${local_svn_repo}
    URL ${CMAKE_CURRENT_SOURCE_DIR}/svnrepo.tgz
    URL_MD5 2f468be4ed1fa96377fca0cc830819c4
    BUILD_COMMAND ""
    CONFIGURE_COMMAND "${Subversion_SVN_EXECUTABLE}" --version
    INSTALL_COMMAND ""
  )
  set_property(TARGET ${proj}
    PROPERTY FOLDER "SetupRepos/Local/Deeply/Nested/For/Testing")

  # SVN by date stamp:
  #
  set(proj TutorialStep1-SVN-20090626)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "${local_svn_repo_url}"
    SVN_REVISION "-r{2009-06-26 16:50:00 +0000}"
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalSVNRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "SVN")

  # SVN by revision number:
  #
  set(proj TutorialStep1-SVN-r2)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "${local_svn_repo_url}"
    SVN_REVISION "-r2"
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalSVNRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "SVN")

  # Live SVN / trunk (no SVN_REVISION):
  #
  set(proj TutorialStep1-SVN-trunk)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "${local_svn_repo_url}"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalSVNRepository"
    LOG_DOWNLOAD 1
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "SVN")
endif()


set(do_git_tests 0)

if(GIT_EXECUTABLE)
  set(do_git_tests 1)

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" --version
    OUTPUT_VARIABLE ov
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  string(REGEX REPLACE "^git version (.+)$" "\\1" git_version "${ov}")
  message(STATUS "git_version='${git_version}'")

  if(git_version VERSION_LESS 1.6.5)
    message(STATUS "No ExternalProject git tests with git client less than version 1.6.5")
    set(do_git_tests 0)
  endif()
endif()


if(do_git_tests)
  set(local_git_repo "../../LocalRepositories/GIT")

  # Unzip/untar the git repository in our source folder so that other
  # projects below may use it to test git args of ExternalProject_Add
  #
  set(proj SetupLocalGITRepository)
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LocalRepositories/GIT
    URL ${CMAKE_CURRENT_SOURCE_DIR}/gitrepo.tgz
    BUILD_COMMAND ""
    CONFIGURE_COMMAND "${GIT_EXECUTABLE}" --version
    INSTALL_COMMAND ""
  )
  set_property(TARGET ${proj}
    PROPERTY FOLDER "SetupRepos/Local/Deeply/Nested/For/Testing")

  # git by commit id:
  #
  set(proj TutorialStep1-GIT-byhash)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${local_git_repo}"
    GIT_TAG d1970730310fe8bc07e73f15dc570071f9f9654a
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalGITRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "GIT")

  # git by explicit branch/tag name:
  #
  set(proj TutorialStep1-GIT-bytag)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${local_git_repo}"
    GIT_TAG "origin/master"
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalGITRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "GIT")

  # Live git / master (no GIT_TAG):
  #
  set(proj TutorialStep1-GIT-master)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${local_git_repo}"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalGITRepository"
    LOG_UPDATE 1
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "GIT")
endif()

set(do_hg_tests 0)

if(HG_EXECUTABLE)
  set(do_hg_tests 1)
endif()

if(do_hg_tests)
  set(local_hg_repo "../../LocalRepositories/HG")

  # Unzip/untar the hg repository in our source folder so that other
  # projects below may use it to test hg args of ExternalProject_Add
  #
  set(proj SetupLocalHGRepository)
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LocalRepositories/HG
    URL ${CMAKE_CURRENT_SOURCE_DIR}/hgrepo.tgz
    BUILD_COMMAND ""
    CONFIGURE_COMMAND "${HG_EXECUTABLE}" --version
    INSTALL_COMMAND ""
  )
  set_property(TARGET ${proj}
    PROPERTY FOLDER "SetupRepos/Local/Deeply/Nested/For/Testing")


  # hg by commit id:
  #
  set(proj TutorialStep1-HG-byhash)
  ExternalProject_Add(${proj}
    HG_REPOSITORY "${local_hg_repo}"
    HG_TAG dd2ce38a6b8a
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalHGRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "HG")

  # hg by explicit branch/tag name:
  #
  set(proj TutorialStep1-HG-bytag)
  ExternalProject_Add(${proj}
    HG_REPOSITORY "${local_hg_repo}"
    HG_TAG "default"
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    DEPENDS "SetupLocalHGRepository"
  )
  set_property(TARGET ${proj} PROPERTY FOLDER "HG")

  # Live hg / tip (no HG_TAG):
  #
  # Mercurial 2.1 does not distinguish an empty pull from a failed pull,
  # so do not run the test with that version.
  if(NOT "${HG_VERSION_STRING}" STREQUAL "2.1")
    set(proj TutorialStep1-HG-tip)
    ExternalProject_Add(${proj}
      HG_REPOSITORY "${local_hg_repo}"
      CMAKE_GENERATOR "${CMAKE_GENERATOR}"
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      INSTALL_COMMAND ""
      DEPENDS "SetupLocalHGRepository"
      LOG_UPDATE 1
    )
    set_property(TARGET ${proj} PROPERTY FOLDER "HG")
  endif()
endif()


# Test the testable built/installed products:
#
enable_testing()


# Do at least a smoke test of a built executable from each
# project's build directory...
#
# BuildTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-Local-BuildTreeTest
    "${binary_base}/TutorialStep5-Local/Tutorial" 42)
  set_property(TEST TutorialStep5-Local-BuildTreeTest
    APPEND PROPERTY LABELS Step5 BuildTree)
endif()

add_test(TutorialStep1-LocalTAR-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalTAR/EP-Tutorial" 36)
set_property(TEST TutorialStep1-LocalTAR-BuildTreeTest
  APPEND PROPERTY LABELS TAR)

add_test(TutorialStep1-LocalNoDirTAR-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalNoDirTAR/EP-Tutorial" 25)

add_test(TutorialStep1-LocalTGZ-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalTGZ/Tutorial" 16)
set_property(TEST TutorialStep1-LocalTGZ-BuildTreeTest
  APPEND PROPERTY LABELS TGZ)

add_test(TutorialStep1-LocalNoDirTGZ-BuildTreeTest
  "${binary_base}/TutorialStep1-LocalNoDirTGZ/Tutorial" 9)

if(do_cvs_tests)
  add_test(TutorialStep1-CVS-20090626-BuildTreeTest
    "${binary_base}/TutorialStep1-CVS-20090626/Tutorial" 4)

  add_test(TutorialStep1-CVS-testtag1-BuildTreeTest
    "${binary_base}/TutorialStep1-CVS-testtag1/Tutorial" 64)

  add_test(TutorialStep1-CVS-HEAD-BuildTreeTest
    "${binary_base}/TutorialStep1-CVS-HEAD/Tutorial" 81)
endif()

if(do_svn_tests)
  add_test(TutorialStep1-SVN-20090626-BuildTreeTest
    "${binary_base}/TutorialStep1-SVN-20090626/Tutorial" 100)

  add_test(TutorialStep1-SVN-r2-BuildTreeTest
    "${binary_base}/TutorialStep1-SVN-r2/Tutorial" 99)

  add_test(TutorialStep1-SVN-trunk-BuildTreeTest
    "${binary_base}/TutorialStep1-SVN-trunk/Tutorial" 98)
endif()

if(do_git_tests)
  add_test(TutorialStep1-GIT-byhash
    "${binary_base}/TutorialStep1-GIT-byhash/Tutorial" 100)

  add_test(TutorialStep1-GIT-bytag
    "${binary_base}/TutorialStep1-GIT-bytag/Tutorial" 99)

  add_test(TutorialStep1-GIT-master
    "${binary_base}/TutorialStep1-GIT-master/Tutorial" 98)
endif()


# InstallTree tests:
#
if(can_build_tutorial_step5)
  add_test(TutorialStep5-InstallTreeTest
    "${TutorialStep5_install_dir}/bin/Tutorial" 49)
  set_property(TEST TutorialStep5-InstallTreeTest
    APPEND PROPERTY LABELS Step5 InstallTree)
endif()


message(STATUS "can_build_tutorial_step5='${can_build_tutorial_step5}'")
message(STATUS "do_cvs_tests='${do_cvs_tests}'")
message(STATUS "do_svn_tests='${do_svn_tests}'")
message(STATUS "do_git_tests='${do_git_tests}' GIT_EXECUTABLE='${GIT_EXECUTABLE}'")
message(STATUS "do_hg_tests='${do_hg_tests}'   HG_EXECUTABLE='${HG_EXECUTABLE}'")
