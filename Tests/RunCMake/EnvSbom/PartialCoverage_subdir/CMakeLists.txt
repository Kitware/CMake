add_library(libC INTERFACE)
add_library(libD INTERFACE)

install(TARGETS libC EXPORT explicit_subdir DESTINATION .)
install(TARGETS libD EXPORT implicit_subdir DESTINATION .)

install(SBOM explicit_subdir_sbom EXPORTS explicit_subdir DESTINATION .)
