set(CPACK_COMPONENTS_ALL test)
install(DIRECTORY DESTINATION empty
        COMPONENT test)

set(CPACK_PACKAGE_NAME "components_empty_dir")
