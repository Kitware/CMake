cmake_minimum_required(VERSION 3.23)


# TODO4: Set the SKIP_TESTS variable to a true value, so that the tests from
#        Exercise1 and Exercise2 are skipped


# TODO5: Include Exercise1.cmake and Exercise2.cmake


set(InList FooBar QuxBar)

# TODO6: Append FooBaz and QuxBaz to InList with FuncAppend


if(NOT InList STREQUAL "FooBar;QuxBar;FooBaz;QuxBaz")
  message(WARNING "Append failed, InList contains: ${InList}")
endif()


# TODO7: Filter InList with FilterFoo, use OutList as the output variable


check_contains(FooBar)
check_contains(FooBaz)
check_nonfoo(${OutList})

if(NOT Failed)
  message("Success!")
endif()
