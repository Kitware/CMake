project(incomplete-genex)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0023 NEW)

add_library(somelib empty.cpp)
target_include_directories(somelib PUBLIC

  "/include;/include$<BUILD_INTERFACE:subdir"
)

export(TARGETS somelib FILE somelibTargets.cmake)

install(TARGETS somelib EXPORT someExport DESTINATION prefix)
install(EXPORT someExport DESTINATION prefix)
