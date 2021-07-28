include(${CMAKE_CURRENT_LIST_DIR}/findAttribute.cmake)

findAttribute(${test} "RemoveHeadersOnCopy" TRUE)
findAttribute(${test} "CodeSignOnCopy" TRUE)
