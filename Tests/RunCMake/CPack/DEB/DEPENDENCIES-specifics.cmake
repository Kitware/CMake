set(CPACK_PACKAGE_CONTACT "someone")
set(CPACK_DEB_COMPONENT_INSTALL "ON")

# false by default
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS FALSE)
# FIXME can not be tested as libraries first have to be part of a package in order
# to determine their dependencies and we can not be certain if there will be any
set(CPACK_DEBIAN_APPLICATIONS_AUTO_PACKAGE_SHLIBDEPS TRUE)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "depend-default, depend-default-b")
set(CPACK_DEBIAN_APPLICATIONS_PACKAGE_DEPENDS "depend-application, depend-application-b")
set(CPACK_DEBIAN_APPLICATIONS_AUTO_PACKAGE_DEPENDS "depend-application, depend-application-b")
set(CPACK_DEBIAN_HEADERS_PACKAGE_DEPENDS "depend-headers")

# TODO add other dependency tests once CPackDeb supports them
