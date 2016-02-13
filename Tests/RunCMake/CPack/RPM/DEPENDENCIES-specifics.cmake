set(CPACK_RPM_COMPONENT_INSTALL "ON")

# FIXME auto autoprov is not tested at the moment as Ubuntu 15.04 rpmbuild
# does not use them correctly: https://bugs.launchpad.net/rpm/+bug/1475755
set(CPACK_RPM_PACKAGE_AUTOREQ "no")
set(CPACK_RPM_PACKAGE_AUTOPROV "no")
set(CPACK_RPM_APPLICATIONS_AUTO_PACKAGE_AUTOREQPROV "yes")
set(CPACK_RPM_LIBS_AUTO_PACKAGE_AUTOREQPROV "yes")

set(CPACK_RPM_PACKAGE_REQUIRES "depend-default, depend-default-b")
set(CPACK_RPM_APPLICATIONS_PACKAGE_REQUIRES "depend-application, depend-application-b")
set(CPACK_RPM_APPLICATIONS_AUTO_PACKAGE_REQUIRES "depend-application, depend-application-b")
set(CPACK_RPM_HEADERS_PACKAGE_REQUIRES "depend-headers")

set(CPACK_RPM_PACKAGE_CONFLICTS "conflict-default, conflict-default-b")
set(CPACK_RPM_APPLICATIONS_PACKAGE_CONFLICTS "conflict-application, conflict-application-b")
set(CPACK_RPM_APPLICATIONS_AUTO_PACKAGE_CONFLICTS "conflict-application, conflict-application-b")
set(CPACK_RPM_HEADERS_PACKAGE_CONFLICTS "conflict-headers")

set(CPACK_RPM_PACKAGE_PROVIDES "provided-default, provided-default-b")
set(CPACK_RPM_LIBS_PACKAGE_PROVIDES "provided-lib")
set(CPACK_RPM_LIBS_AUTO_PACKAGE_PROVIDES "provided-lib_auto, provided-lib_auto-b")
