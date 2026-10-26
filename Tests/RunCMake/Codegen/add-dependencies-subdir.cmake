add_subdirectory(AddDependenciesSubDir EXCLUDE_FROM_ALL)

add_executable(main
    main.c
)

# codegen must reach h_creator in the excluded directory through main.
add_dependencies(main h_creator)
