include(${CMAKE_CURRENT_LIST_DIR}/findAttribute.cmake)

findAttribute(${test} "RemoveHeadersOnCopy" FALSE)
findAttribute(${test} "CodeSignOnCopy" FALSE)
