enable_language(C)

# Model directory-scoped platform settings.  Only the child directory contains
# the archive, and the executable links by name so its search path is required.
set(CMAKE_C_STANDARD_LINK_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/parent-only")
add_subdirectory(XcodeStandardLinkDirectoriesSubdir)
