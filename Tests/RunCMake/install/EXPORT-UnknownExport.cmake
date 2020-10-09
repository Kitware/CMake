enable_language(C)
install(EXPORT fooExport
    DESTINATION "lib/cmake/"
    FILE "foo.cmake"
)
