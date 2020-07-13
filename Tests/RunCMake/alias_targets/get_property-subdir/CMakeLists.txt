

add_library(alias::import-local-subdir ALIAS import-local)

check_property (alias::import-local-subdir ALIASED_TARGET "import-local")
check_property (alias::import-local-subdir IMPORTED "TRUE")
check_property (alias::import-local-subdir ALIAS_GLOBAL "FALSE")
check_property (alias::import-local-subdir IMPORT_LOCAL_PROPERTY "IMPORT_LOCAL")


# non-global alias defined in parent directory must be visible in sub-directory
check_property (alias::import-local ALIASED_TARGET "import-local")
check_property (alias::import-local IMPORTED "TRUE")
check_property (alias::import-local ALIAS_GLOBAL "FALSE")
check_property (alias::import-local IMPORT_LOCAL_PROPERTY "IMPORT_LOCAL")
