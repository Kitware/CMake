project(VSCSharpReference)

include_external_msproject(external external.csproj)

add_executable(internal
    main.cpp
)
add_dependencies(internal
    external
)
