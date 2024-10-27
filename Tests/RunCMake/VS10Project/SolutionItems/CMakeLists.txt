cmake_minimum_required(VERSION 3.10)
project(SolutionItemsSubproject)
set_property(DIRECTORY APPEND PROPERTY VS_SOLUTION_ITEMS extraneous.txt)
source_group("Extraneous" REGULAR_EXPRESSION "[^.]+\\.txt")
