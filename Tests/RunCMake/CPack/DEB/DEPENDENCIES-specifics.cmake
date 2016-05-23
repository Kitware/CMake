set(CPACK_PACKAGE_CONTACT "someone")
set(CPACK_DEB_COMPONENT_INSTALL "ON")
#intentionaly commented out to test old file naming
#set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# false by default
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS FALSE)
# FIXME can not be tested as libraries first have to be part of a package in order
# to determine their dependencies and we can not be certain if there will be any
set(CPACK_DEBIAN_APPLICATIONS_AUTO_PACKAGE_SHLIBDEPS TRUE)

foreach(dependency_type_ DEPENDS CONFLICTS PREDEPENDS ENHANCES BREAKS REPLACES RECOMMENDS SUGGESTS)
  string(TOLOWER "${dependency_type_}" lower_dependency_type_)

  set(CPACK_DEBIAN_PACKAGE_${dependency_type_} "${lower_dependency_type_}-default, ${lower_dependency_type_}-default-b")
  set(CPACK_DEBIAN_APPLICATIONS_PACKAGE_${dependency_type_} "${lower_dependency_type_}-application, ${lower_dependency_type_}-application-b")
  set(CPACK_DEBIAN_APPLICATIONS_AUTO_PACKAGE_${dependency_type_} "${lower_dependency_type_}-application, ${lower_dependency_type_}-application-b")
  set(CPACK_DEBIAN_HEADERS_PACKAGE_${dependency_type_} "${lower_dependency_type_}-headers")
endforeach()

set(CPACK_DEBIAN_PACKAGE_PROVIDES "provided-default, provided-default-b")
set(CPACK_DEBIAN_LIBS_PACKAGE_PROVIDES "provided-lib")
set(CPACK_DEBIAN_LIBS_AUTO_PACKAGE_PROVIDES "provided-lib_auto, provided-lib_auto-b")
