add_library(Scoped::dep INTERFACE IMPORTED)
set_target_properties(Scoped::dep PROPERTIES
  EXPORT_FIND_PACKAGE_NAME "${dependency_package}"
  SPDX_LICENSE "${dependency_license}"
)

add_library(scoped_${dependency_package} INTERFACE)
target_link_libraries(scoped_${dependency_package} INTERFACE Scoped::dep)

target_link_libraries(root INTERFACE Scoped::dep)
install(TARGETS scoped_${dependency_package}
  EXPORT unknown_dependencies DESTINATION .)
