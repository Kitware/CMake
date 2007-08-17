# CPack script for creating RPM package
# Author: Eric Noulard with the help of Alexander Neundorf

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

# RPM_PACKAGE_SUMMARY (mandatory)
IF(NOT RPM_PACKAGE_SUMMARY)
  # if neither var is defined lets use the name as summary
  IF(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    STRING(TOLOWER "${CPACK_PACKAGE_NAME}" RPM_PACKAGE_SUMMARY)
  ELSE(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    SET(RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})    
  ENDIF(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
ENDIF(NOT RPM_PACKAGE_SUMMARY)
 
# RPM_PACKAGE_NAME (mandatory)
IF(NOT RPM_PACKAGE_NAME)
  STRING(TOLOWER "${CPACK_PACKAGE_NAME}" RPM_PACKAGE_NAME)
ENDIF(NOT RPM_PACKAGE_NAME)

# RPM_PACKAGE_VERSION (mandatory)
IF(NOT RPM_PACKAGE_VERSION)
  IF(NOT CPACK_PACKAGE_VERSION)
    MESSAGE(FATAL_ERROR "RPM package requires a package version")
  ENDIF(NOT CPACK_PACKAGE_VERSION)
  SET(RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
ENDIF(NOT RPM_PACKAGE_VERSION)

# RPM_PACKAGE_ARCHITECTURE (optional)
IF(NOT RPM_PACKAGE_ARCHITECTURE)
  # FIXME This should be obtained through 'arch' command
  # but is it --really necessary-- ?
  SET(RPM_PACKAGE_ARCHITECTURE i386)
ENDIF(NOT RPM_PACKAGE_ARCHITECTURE)

# RPM_PACKAGE_RELEASE
# The RPM release is the numbering of the RPM package ITSELF
# this is the version of the PACKAGING and NOT the version
# of the CONTENT of the package.
# You may well need to generate a new RPM package release
# without changing the version of the packaged software.
# This is the case when the packaging is buggy (not) the software :=)
# If not set, 1 is a good candidate
IF(NOT RPM_PACKAGE_RELEASE)
  SET(RPM_PACKAGE_RELEASE 1)
ENDIF(NOT RPM_PACKAGE_RELEASE)

# RPM_PACKAGE_LICENSE
IF(NOT RPM_PACKAGE_LICENSE)
  SET(RPM_PACKAGE_LICENSE "unknown")
ENDIF(NOT RPM_PACKAGE_LICENSE)

# RPM_PACKAGE_GROUP
IF(NOT RPM_PACKAGE_GROUP)
  SET(RPM_PACKAGE_GROUP "unknown")
ENDIF(NOT RPM_PACKAGE_GROUP)

# RPM_PACKAGE_SOURCE
# The name of the source tarball in case we generate
# a source RPM

# RPM_PACKAGE_DESCRIPTION
# FIXME may be found in CPACK_PACKAGE_DESCRIPTION_FILE

# RPM_USER_BINARY_SPECFILE 
# FIXME when this is set then CPack should us the 
# user provided file.

# Now we may create the RPM build tree structure
SET(RPM_ROOTDIR "${CPACK_TOPLEVEL_DIRECTORY}")
MESSAGE(STATUS "CPackRPM:: Using RPM_ROOTDIR=${RPM_ROOTDIR}")
# Prepare RPM build tree
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR})
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/tmp)
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/BUILD)
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/RPMS)
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/SOURCES)
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/SPECS)
FILE(MAKE_DIRECTORY ${RPM_ROOTDIR}/SRPMS)

#SET(RPM_FILE_NAME "${RPM_PACKAGE_NAME}-${RPM_PACKAGE_VERSION}-${RPM_PACKAGE_RELEASE}-${RPM_PACKAGE_ARCHITECTURE}.rpm")
SET(RPM_FILE_NAME "${CPACK_OUTPUT_FILE_NAME}")
# it seems rpmbuild can't handle spaces in the path
# neither escaping (as below) nor putting quotes around the path seem to help
#STRING(REGEX REPLACE " " "\\\\ " RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
SET(RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
  

SET(RPM_BINARY_SPECFILE "${RPM_ROOTDIR}/SPECS/${RPM_PACKAGE_NAME}.spec")
IF(RPM_USER_BINARY_SPECFILE)
  # User may have specified SPECFILE just use it
  MESSAGE("CPackRPM: Will use user specified spec file: ${RPM_USER_BINARY_SPECFILE}")
  # Note that user provided file is processed for @var replacement
  CONFIGURE_FILE(${RPM_USER_BINARY_SPECFILE} ${RPM_BINARY_SPECFILE} @ONLY)
ELSE(RPM_USER_BINARY_SPECFILE)
  # No User specified spec file generate a valid one using var values
  IF(RPM_PACKAGE_DEBUG)
    MESSAGE("CPackRPM:: CPACK_TOPLEVEL_DIRECTORY = ${CPACK_TOPLEVEL_DIRECTORY}")
    MESSAGE("CPackRPM:: CPACK_TOPLEVEL_TAG       = ${CPACK_TOPLEVEL_TAG}")
    MESSAGE("CPackRPM:: CPACK_TEMPORARY_DIRECTORY= ${PACK_TEMPORARY_DIRECTORY}")
    MESSAGE("CPackRPM:: CPACK_OUTPUT_FILE_NAME   = ${CPACK_OUTPUT_FILE_NAME}")
    MESSAGE("CPackRPM:: CPACK_OUTPUT_FILE_PATH   = ${CPACK_OUTPUT_FILE_PATH}")
    MESSAGE("CPackRPM:: CPACK_PACKAGE_FILE_NAME  = ${CPACK_PACKAGE_FILE_NAME}")
    MESSAGE("CPackRPM:: RPM_BINARY_SPECFILE      = ${RPM_BINARY_SPECFILE}")
    MESSAGE("CPackRPM:: CPACK_PACKAGE_INSTALL_DIRECTORY   = ${PACK_PACKAGE_INSTALL_DIRECTORY}")
    MESSAGE("CPackRPM ::CPACK_TEMPORARY_PACKAGE_FILE_NAME = ${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
  ENDIF(RPM_PACKAGE_DEBUG)
  FILE(WRITE ${RPM_BINARY_SPECFILE}
    "# -*- rpm-spec -*-
Buildroot:      ${RPM_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
Summary:        ${RPM_PACKAGE_SUMMARY}
Name:           ${RPM_PACKAGE_NAME}
Version:        ${RPM_PACKAGE_VERSION}
Release:        ${RPM_PACKAGE_RELEASE}
License:        ${RPM_PACKAGE_LICENSE}
Group:          ${RPM_PACKAGE_LICENSE}

#%define prefix ${CMAKE_INSTALL_PREFIX}
%define _rpmdir ${RPM_DIRECTORY}
%define _rpmfilename ${RPM_FILE_NAME}
%define _unpackaged_files_terminate_build 0
%define _topdir ${RPM_DIRECTORY}

%description
${RPM_PACKAGE_NAME} : will come soon 
with the value of RPM_PACKAGE_DESCRIPTION
or
the content of CPACK_PACKAGE_DESCRIPTION_FILE
***
${RPM_PACKAGE_DESCRIPTION}
***

%prep

%build
  
%install

%clean

%files
%defattr(-,root,root,-)
#%dir %{prefix}
#%{prefix}/*
/*

%changelog
* Tue Aug 16 2007 Erk <eric.noulard@gmail.com>
  Generated by CPack RPM Generator and associated macros
")

ENDIF(RPM_USER_BINARY_SPECFILE)


IF(RPMBUILD_EXECUTABLE)
  # Now call rpmbuild using the SPECFILE
  EXECUTE_PROCESS(
    COMMAND "${RPMBUILD_EXECUTABLE}" -bb "${RPM_BINARY_SPECFILE}"
    WORKING_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}"
    ERROR_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.err"
    OUTPUT_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild.out")
ELSE(RPMBUILD_EXECUTABLE)
  IF(ALIEN_EXECUTABLE)
    MESSAGE(FATAL_ERROR "RPM packaging through alien not done (yet)")
  ENDIF(ALIEN_EXECUTABLE)
ENDIF(RPMBUILD_EXECUTABLE)
