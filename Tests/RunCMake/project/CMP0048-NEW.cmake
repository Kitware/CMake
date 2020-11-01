include(PrintVersions.cmake)

cmake_policy(SET CMP0048 NEW)

project(ProjectA VERSION 1.2.3.4 LANGUAGES NONE)
print_versions(ProjectA)

project(ProjectB VERSION 0.1.2 LANGUAGES NONE)
print_versions(ProjectB)

set(PROJECT_VERSION 1)
set(ProjectC_VERSION 1)
project(ProjectC NONE)
print_versions(ProjectC)
