# Test moc include patterns

set(CMAKE_AUTOMOC_RELAXED_MODE FALSE)
set(MOC_INCLUDE_NAME "mocIncludeStrict")

include(${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/shared.cmake)
