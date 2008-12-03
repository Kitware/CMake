# CPack script for creating RPM package
# Author: Eric Noulard with the help of Alexander Neundorf.
# All variables used by CPackRPM begins with CPACK_RPM_ prefix
#
# Here comes the list of used variables:
#

IF(CMAKE_BINARY_DIR)
  MESSAGE(FATAL_ERROR "CPackRPM.cmake may only be used by CPack internally.")
ENDIF(CMAKE_BINARY_DIR)

IF(NOT UNIX)
  MESSAGE(FATAL_ERROR "CPackRPM.cmake may only be used under UNIX.")
ENDIF(NOT UNIX)

# rpmbuild is the basic command for building RPM package
# it may be a simple (symbolic) link to rpmb command.
FIND_PROGRAM(RPMBUILD_EXECUTABLE rpmbuild)

IF(NOT RPMBUILD_EXECUTABLE)
  MESSAGE(FATAL_ERROR "RPM package requires rpmbuild executable")
ENDIF(NOT RPMBUILD_EXECUTABLE)

IF(CPACK_TOPLEVEL_DIRECTORY MATCHES ".* .*")
  MESSAGE(FATAL_ERROR "${RPMBUILD_EXECUTABLE} can't handle paths with spaces, use a build directory without spaces for building RPMs.")
ENDIF(CPACK_TOPLEVEL_DIRECTORY MATCHES ".* .*")

# If rpmbuild is found 
# we try to discover alien since we may be on non RPM distro like Debian.
# In this case we may try to to use more advanced features
# like generating RPM directly from DEB using alien.
# FIXME feature not finished (yet)
FIND_PROGRAM(ALIEN_EXECUTABLE alien)
IF(ALIEN_EXECUTABLE)
  MESSAGE(STATUS "alien found, we may be on a Debian based distro.")
ENDIF(ALIEN_EXECUTABLE)

# 
# Use user-defined RPM specific variables value
# or generate reasonable default value from
# CPACK_xxx generic values.
# The variables comes from the needed (mandatory or not)
# values found in the RPM specification file aka ".spec" file.
# The variables which may/should be defined are:
#

# CPACK_RPM_PACKAGE_SUMMARY (mandatory)
IF(NOT CPACK_RPM_PACKAGE_SUMMARY)
  # if neither var is defined lets use the name as summary
  IF(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    STRING(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_SUMMARY)
  ELSE(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    SET(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})    
  ENDIF(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
ENDIF(NOT CPACK_RPM_PACKAGE_SUMMARY)
 
# CPACK_RPM_PACKAGE_NAME (mandatory)
IF(NOT CPACK_RPM_PACKAGE_NAME)
  STRING(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_NAME)
ENDIF(NOT CPACK_RPM_PACKAGE_NAME)

# CPACK_RPM_PACKAGE_VERSION (mandatory)
IF(NOT CPACK_RPM_PACKAGE_VERSION)
  IF(NOT CPACK_PACKAGE_VERSION)
    MESSAGE(FATAL_ERROR "RPM package requires a package version")
  ENDIF(NOT CPACK_PACKAGE_VERSION)
  SET(CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
ENDIF(NOT CPACK_RPM_PACKAGE_VERSION)

# CPACK_RPM_PACKAGE_ARCHITECTURE (optional)
IF(CPACK_RPM_PACKAGE_ARCHITECTURE)
  SET(TMP_RPM_BUILDARCH "Buildarch: ${CPACK_RPM_PACKAGE_ARCHITECTURE}")
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: using user-specified build arch = ${CPACK_RPM_PACKAGE_ARCHITECTURE}")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
ELSE(CPACK_RPM_PACKAGE_ARCHITECTURE)
  SET(TMP_RPM_BUILDARCH "")
ENDIF(CPACK_RPM_PACKAGE_ARCHITECTURE)

# CPACK_RPM_PACKAGE_RELEASE
# The RPM release is the numbering of the RPM package ITSELF
# this is the version of the PACKAGING and NOT the version
# of the CONTENT of the package.
# You may well need to generate a new RPM package release
# without changing the version of the packaged software.
# This is the case when the packaging is buggy (not) the software :=)
# If not set, 1 is a good candidate
IF(NOT CPACK_RPM_PACKAGE_RELEASE)
  SET(CPACK_RPM_PACKAGE_RELEASE 1)
ENDIF(NOT CPACK_RPM_PACKAGE_RELEASE)

# CPACK_RPM_PACKAGE_LICENSE
IF(NOT CPACK_RPM_PACKAGE_LICENSE)
  SET(CPACK_RPM_PACKAGE_LICENSE "unknown")
ENDIF(NOT CPACK_RPM_PACKAGE_LICENSE)

# CPACK_RPM_PACKAGE_GROUP
IF(NOT CPACK_RPM_PACKAGE_GROUP)
  SET(CPACK_RPM_PACKAGE_GROUP "unknown")
ENDIF(NOT CPACK_RPM_PACKAGE_GROUP)

# CPACK_RPM_PACKAGE_VENDOR
IF(NOT CPACK_RPM_PACKAGE_VENDOR)
  IF(CPACK_PACKAGE_VENDOR)
    SET(CPACK_RPM_PACKAGE_VENDOR "${CPACK_PACKAGE_VENDOR}")
  ELSE(CPACK_PACKAGE_VENDOR)
    SET(CPACK_RPM_PACKAGE_VENDOR "unknown")
  ENDIF(CPACK_PACKAGE_VENDOR)
ENDIF(NOT CPACK_RPM_PACKAGE_VENDOR)

# CPACK_RPM_PACKAGE_SOURCE
# The name of the source tarball in case we generate a source RPM

# CPACK_RPM_PACKAGE_DESCRIPTION
# The variable content may be either
#   - explicitely given by tthe user or
#   - filled with the content of CPACK_PACKAGE_DESCRIPTION_FILE
#     if it is defined
#   - set to a default value
#
IF (NOT CPACK_RPM_PACKAGE_DESCRIPTION)
        IF (CPACK_PACKAGE_DESCRIPTION_FILE)
                FILE(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_RPM_PACKAGE_DESCRIPTION)
        ELSE (CPACK_PACKAGE_DESCRIPTION_FILE)
                SET(CPACK_RPM_PACKAGE_DESCRIPTION "no package description available")
        ENDIF (CPACK_PACKAGE_DESCRIPTION_FILE)
ENDIF (NOT CPACK_RPM_PACKAGE_DESCRIPTION)

# CPACK_RPM_PACKAGE_REQUIRES
# Placeholder used to specify binary RPM dependencies (if any)
# see http://www.rpm.org/max-rpm/s1-rpm-depend-manual-dependencies.html
IF(CPACK_RPM_PACKAGE_REQUIRES)
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: User defined Requires:\n ${CPACK_RPM_PACKAGE_REQUIRES}")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
  SET(TMP_RPM_REQUIRES "Requires: ${CPACK_RPM_PACKAGE_REQUIRES}")
ENDIF(CPACK_RPM_PACKAGE_REQUIRES)

# CPACK_RPM_SPEC_INSTALL_POST
# May be used to define a RPM post intallation script
# for example setting it to "/bin/true" may prevent
# rpmbuild from stripping binaries.
IF(CPACK_RPM_SPEC_INSTALL_POST)
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: User defined CPACK_RPM_SPEC_INSTALL_POST = ${CPACK_RPM_SPEC_INSTALL_POST}")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
  SET(TMP_RPM_SPEC_INSTALL_POST "%define __spec_install_post ${CPACK_RPM_SPEC_INSTALL_POST}")
ENDIF(CPACK_RPM_SPEC_INSTALL_POST)

# CPACK_RPM_SPEC_MORE_DEFINE
# This is a generated spec rpm file spaceholder
IF(CPACK_RPM_SPEC_MORE_DEFINE)
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: User defined more define spec line specified:\n ${CPACK_RPM_SPEC_MORE_DEFINE}")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
ENDIF(CPACK_RPM_SPEC_MORE_DEFINE)

# CPACK_RPM_USER_BINARY_SPECFILE 
# FIXME when this is set then CPack should us the 
# user provided file.

# Now we may create the RPM build tree structure
SET(CPACK_RPM_ROOTDIR "${CPACK_TOPLEVEL_DIRECTORY}")
MESSAGE(STATUS "CPackRPM:Debug: Using CPACK_RPM_ROOTDIR=${CPACK_RPM_ROOTDIR}")
# Prepare RPM build tree
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR})
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/tmp)
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/BUILD)
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/RPMS)
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SOURCES)
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SPECS)
FILE(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SRPMS)

#SET(CPACK_RPM_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}-${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
SET(CPACK_RPM_FILE_NAME "${CPACK_OUTPUT_FILE_NAME}")
# it seems rpmbuild can't handle spaces in the path
# neither escaping (as below) nor putting quotes around the path seem to help
#STRING(REGEX REPLACE " " "\\\\ " CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
SET(CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
  

SET(CPACK_RPM_BINARY_SPECFILE "${CPACK_RPM_ROOTDIR}/SPECS/${CPACK_RPM_PACKAGE_NAME}.spec")
IF(CPACK_RPM_USER_BINARY_SPECFILE)
  # User may have specified SPECFILE just use it
  MESSAGE("CPackRPM: Will use user specified spec file: ${CPACK_RPM_USER_BINARY_SPECFILE}")
  # Note that user provided file is processed for @var replacement
  CONFIGURE_FILE(${CPACK_RPM_USER_BINARY_SPECFILE} ${CPACK_RPM_BINARY_SPECFILE} @ONLY)
ELSE(CPACK_RPM_USER_BINARY_SPECFILE)
  # No User specified spec file generate a valid one using var values
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: CPACK_TOPLEVEL_DIRECTORY          = ${CPACK_TOPLEVEL_DIRECTORY}")
    MESSAGE("CPackRPM:Debug: CPACK_TOPLEVEL_TAG                = ${CPACK_TOPLEVEL_TAG}")
    MESSAGE("CPackRPM:Debug: CPACK_TEMPORARY_DIRECTORY         = ${CPACK_TEMPORARY_DIRECTORY}")
    MESSAGE("CPackRPM:Debug: CPACK_OUTPUT_FILE_NAME            = ${CPACK_OUTPUT_FILE_NAME}")
    MESSAGE("CPackRPM:Debug: CPACK_OUTPUT_FILE_PATH            = ${CPACK_OUTPUT_FILE_PATH}")
    MESSAGE("CPackRPM:Debug: CPACK_PACKAGE_FILE_NAME           = ${CPACK_PACKAGE_FILE_NAME}")
    MESSAGE("CPackRPM:Debug: CPACK_RPM_BINARY_SPECFILE         = ${CPACK_RPM_BINARY_SPECFILE}")
    MESSAGE("CPackRPM:Debug: CPACK_PACKAGE_INSTALL_DIRECTORY   = ${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    MESSAGE("CPackRPM:Debug: CPACK_TEMPORARY_PACKAGE_FILE_NAME = ${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
  FILE(WRITE ${CPACK_RPM_BINARY_SPECFILE}
    "# -*- rpm-spec -*-
Buildroot:      ${CPACK_RPM_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
Summary:        ${CPACK_RPM_PACKAGE_SUMMARY}
Name:           ${CPACK_RPM_PACKAGE_NAME}
Version:        ${CPACK_RPM_PACKAGE_VERSION}
Release:        ${CPACK_RPM_PACKAGE_RELEASE}
License:        ${CPACK_RPM_PACKAGE_LICENSE}
Group:          ${CPACK_RPM_PACKAGE_GROUP}
Vendor:         ${CPACK_RPM_PACKAGE_VENDOR}
${TMP_RPM_REQUIRES}
${TMP_RPM_BUILDARCH}

#%define prefix ${CMAKE_INSTALL_PREFIX}
%define _rpmdir ${CPACK_RPM_DIRECTORY}
%define _rpmfilename ${CPACK_RPM_FILE_NAME}
%define _unpackaged_files_terminate_build 0
%define _topdir ${CPACK_RPM_DIRECTORY}
${TMP_RPM_SPEC_INSTALL_POST}
${CPACK_RPM_SPEC_MORE_DEFINE}

%description
${CPACK_RPM_PACKAGE_DESCRIPTION}

# This is a shortcutted spec file
# generated by CMake RPM generator
# we skip the _prepn _build and _install
# steps because CPack does that for us
#%prep

#%build
  
#%install

%clean

%files
%defattr(-,root,root,-)
#%dir %{prefix}
#%{prefix}/*
/*

%changelog
* Mon Oct 03 2008 Erk <eric.noulard@gmail.com>
  Update generator to handle optional dependencies using Requires
  Update DEBUG output typos. 
* Mon Aug 25 2008 Erk <eric.noulard@gmail.com>
  Update generator to handle optional post-install
* Tue Aug 16 2007 Erk <eric.noulard@gmail.com>
  Generated by CPack RPM Generator and associated macros
")
ENDIF(CPACK_RPM_USER_BINARY_SPECFILE)


IF(RPMBUILD_EXECUTABLE)
  # Now call rpmbuild using the SPECFILE
  EXECUTE_PROCESS(
    COMMAND "${RPMBUILD_EXECUTABLE}" -bb "${CPACK_RPM_BINARY_SPECFILE}"
    WORKING_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}"
    ERROR_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.err"
    OUTPUT_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.out")
  IF(CPACK_RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:Debug: You may consult rpmbuild logs in: ")
    MESSAGE("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.err")
    MESSAGE("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.out")
  ENDIF(CPACK_RPM_PACKAGE_DEBUG)
ELSE(RPMBUILD_EXECUTABLE)
  IF(ALIEN_EXECUTABLE)
    MESSAGE(FATAL_ERROR "RPM packaging through alien not done (yet)")
  ENDIF(ALIEN_EXECUTABLE)
ENDIF(RPMBUILD_EXECUTABLE)
