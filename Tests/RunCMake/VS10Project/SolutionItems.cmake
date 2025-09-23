set_property(DIRECTORY APPEND PROPERTY VS_SOLUTION_ITEMS
             solution-item-0-1.txt
             solution-item-1-1.txt
             "${CMAKE_CURRENT_LIST_DIR}/solution-item-2-1.txt"
             "${CMAKE_CURRENT_LIST_DIR}/solution-item-2-2.txt")
source_group("Outer Group" FILES solution-item-1-1.txt)
source_group("Outer Group/Inner Group" REGULAR_EXPRESSION "solution-item-2-[0-9]+\\.txt")
add_subdirectory(SolutionItems)
