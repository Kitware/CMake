project(VSCSharpOnlyProject)

file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/Program.cs
    ${CMAKE_CURRENT_SOURCE_DIR}/consoleapp.csproj
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_external_msproject(
    test "${CMAKE_CURRENT_BINARY_DIR}/consoleapp.csproj")
