function(MapAppend map key value)
  list(APPEND "${map}_k" "${key}")
  list(APPEND "${map}_v" "${value}")

  set("${map}_k" "${${map}_k}" PARENT_SCOPE)
  set("${map}_v" "${${map}_v}" PARENT_SCOPE)
endfunction()

function(MapLength map out_variable)
  list(LENGTH "${map}_k" length)
  set("${out_variable}" "${length}" PARENT_SCOPE)
endfunction()

function(MapFind map key out_variable)
  list(FIND "${map}_k" "${key}" index)
  if("${index}" LESS 0)
    unset("${out_variable}" PARENT_SCOPE)
  else()
    list(GET "${map}_v" "${index}" value)
    set("${out_variable}" "${value}" PARENT_SCOPE)
  endif()
endfunction()

macro(MapPropagateToParentScope map)
  set("${map}_k" "${${map}_k}" PARENT_SCOPE)
  set("${map}_v" "${${map}_v}" PARENT_SCOPE)
endmacro()


function(ParseSln vcSlnFile)
  if(NOT EXISTS "${vcSlnFile}")
    set(RunCMake_TEST_FAILED "Solution file ${vcSlnFile} does not exist." PARENT_SCOPE)
    return()
  endif()

  set(SCOPE "")

  set(IN_SOLUTION_ITEMS FALSE)
  set(IN_NESTED_PROJECTS FALSE)

  set(REGUID "\\{[0-9A-F-]+\\}")

  file(STRINGS "${vcSlnFile}" lines)
  foreach(line IN LISTS lines)
    string(STRIP "${line}" line)

    # Project(...)
    if(line MATCHES "Project\\(\"(${REGUID})\"\\) = \"([^\"]+)\", \"([^\"]+)\", \"(${REGUID})\"")
      if(NOT "${SCOPE}" STREQUAL "")
        set(RunCMake_TEST_FAILED "Improper nesting of Project" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "Project")

      if("${CMAKE_MATCH_1}" STREQUAL "{2150E333-8FDC-42A3-9474-1A3956D46DE8}")
        set(GROUP_NAME "${CMAKE_MATCH_2}")

        MapFind(GROUP_PATHS "${GROUP_NAME}" existing_path)
        if(DEFINED existing_path)
          set(RunCMake_TEST_FAILED "Duplicate solution items project '${GROUP_NAME}'" PARENT_SCOPE)
          return()
        endif()

        MapAppend(GROUP_PATHS "${GROUP_NAME}" "${CMAKE_MATCH_3}")
        MapAppend(GROUP_GUIDS "${GROUP_NAME}" "${CMAKE_MATCH_4}")
      endif()

    # EndProject
    elseif(line STREQUAL "EndProject")
      if(NOT "${SCOPE}" STREQUAL "Project")
        set(RunCMake_TEST_FAILED "Improper nesting of EndProject" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "")

      unset(GROUP_NAME)

    # ProjectSection
    elseif(line MATCHES "ProjectSection\\(([a-zA-Z]+)\\) = ([a-zA-Z]+)")
      if(NOT "${SCOPE}" STREQUAL "Project")
        set(RunCMake_TEST_FAILED "Improper nesting of ProjectSection" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "ProjectSection")

      if("${CMAKE_MATCH_1}" STREQUAL "SolutionItems")
        if(NOT "${CMAKE_MATCH_2}" STREQUAL "preProject")
          set(RunCMake_TEST_FAILED "SolutionItems must be preProject" PARENT_SCOPE)
          return()
        endif()

        set(IN_SOLUTION_ITEMS TRUE)
      endif()

    # EndProjectSection
    elseif(line STREQUAL "EndProjectSection")
      if(NOT "${SCOPE}" STREQUAL "ProjectSection")
        set(RunCMake_TEST_FAILED "Improper nesting of EndProjectSection" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "Project")

      set(IN_SOLUTION_ITEMS FALSE)

    # Global
    elseif(line STREQUAL "Global")
      if(NOT "${SCOPE}" STREQUAL "")
        set(RunCMake_TEST_FAILED "Improper nesting of Global" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "Global")

    # EndGlobal
    elseif(line STREQUAL "EndGlobal")
      if(NOT "${SCOPE}" STREQUAL "Global")
        set(RunCMake_TEST_FAILED "Improper nesting of EndGlobal" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "")

    # GlobalSection
    elseif(line MATCHES "GlobalSection\\(([a-zA-Z]+)\\) = ([a-zA-Z]+)")
      if(NOT "${SCOPE}" STREQUAL "Global")
        set(RunCMake_TEST_FAILED "Improper nesting of GlobalSection" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "GlobalSection")

      if("${CMAKE_MATCH_1}" STREQUAL "NestedProjects")
        if(NOT "${CMAKE_MATCH_2}" STREQUAL "preSolution")
          set(RunCMake_TEST_FAILED "NestedProjects must be preSolution" PARENT_SCOPE)
          return()
        endif()

        set(IN_NESTED_PROJECTS TRUE)
      endif()

    # EndGlobalSection
    elseif(line STREQUAL "EndGlobalSection")
      if(NOT "${SCOPE}" STREQUAL "GlobalSection")
        set(RunCMake_TEST_FAILED "Improper nesting of EndGlobalSection" PARENT_SCOPE)
        return()
      endif()
      set(SCOPE "Global")

      set(IN_NESTED_PROJECTS FALSE)

    # .../solution-item-0-1.txt = .../solution-item-0-1.txt
    elseif(${IN_SOLUTION_ITEMS})
      if(NOT line MATCHES "([^=]+)=([^=]+)")
        set(RunCMake_TEST_FAILED "Invalid solution item paths 1" PARENT_SCOPE)
        return()
      endif()

      string(STRIP "${CMAKE_MATCH_1}" CMAKE_MATCH_1)
      string(STRIP "${CMAKE_MATCH_2}" CMAKE_MATCH_2)

      if(NOT "${CMAKE_MATCH_1}" STREQUAL "${CMAKE_MATCH_2}")
        set(RunCMake_TEST_FAILED "Invalid solution item paths 2" PARENT_SCOPE)
        return()
      endif()

      cmake_path(GET CMAKE_MATCH_1 FILENAME filename)
      MapAppend(SOLUTION_ITEMS "${filename}" "${GROUP_NAME}")

    # {1EB55F5E...} = {A11E84C6...}
    elseif(${IN_NESTED_PROJECTS})
      if(NOT line MATCHES "(${REGUID}) = (${REGUID})")
        set(RunCMake_TEST_FAILED "Invalid nested project guids" PARENT_SCOPE)
        return()
      endif()

      MapFind(PROJECT_PARENTS "${CMAKE_MATCH_1}" existing_parent)
      if(DEFINED existing_parent)
        set(RunCMake_TEST_FAILED "Duplicate nested project: '${CMAKE_MATCH_1}'" PARENT_SCOPE)
        return()
      endif()

      MapAppend(PROJECT_PARENTS "${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")

    endif()

    MapPropagateToParentScope(GROUP_PATHS)
    MapPropagateToParentScope(GROUP_GUIDS)
    MapPropagateToParentScope(PROJECT_PARENTS)
    MapPropagateToParentScope(SOLUTION_ITEMS)
  endforeach()
endfunction()


# Check the root solution:
block()
  ParseSln("${RunCMake_TEST_BINARY_DIR}/SolutionItems.sln")

  if(DEFINED RunCMake_TEST_FAILED)
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return()
  endif()


  # Check group guids and nesting:

  MapFind(GROUP_GUIDS "Solution Items" root_group_guid)
  if(NOT DEFINED root_group_guid)
    set(RunCMake_TEST_FAILED "Solution Items not found" PARENT_SCOPE)
    return()
  endif()
  MapFind(GROUP_PATHS "Solution Items" root_group_path)
  if(NOT "${root_group_path}" STREQUAL "Solution Items")
    set(RunCMake_TEST_FAILED "Invalid Solution Items path: '${root_group_path}'" PARENT_SCOPE)
    return()
  endif()
  MapFind(PROJECT_PARENTS "${root_group_guid}" root_group_parent_guid)
  if(DEFINED root_group_parent_guid)
    set(RunCMake_TEST_FAILED "Solution Items is nested" PARENT_SCOPE)
    return()
  endif()

  MapFind(GROUP_GUIDS "Outer Group" outer_group_guid)
  if(NOT DEFINED outer_group_guid)
    set(RunCMake_TEST_FAILED "Outer Group not found" PARENT_SCOPE)
    return()
  endif()
  MapFind(GROUP_PATHS "Outer Group" outer_group_path)
  if(NOT "${outer_group_path}" STREQUAL "Outer Group")
    set(RunCMake_TEST_FAILED "Invalid Outer Group path: '${outer_group_path}'" PARENT_SCOPE)
    return()
  endif()
  MapFind(PROJECT_PARENTS "${outer_group_guid}" outer_group_parent_guid)
  if(DEFINED outer_group_parent_guid)
    set(RunCMake_TEST_FAILED "Outer Group is nested" PARENT_SCOPE)
    return()
  endif()

  MapFind(GROUP_GUIDS "Inner Group" inner_group_guid)
  if(NOT DEFINED inner_group_guid)
    set(RunCMake_TEST_FAILED "Inner Group not found" PARENT_SCOPE)
    return()
  endif()
  MapFind(GROUP_PATHS "Inner Group" inner_group_path)
  if(NOT "${inner_group_path}" STREQUAL "Outer Group\\Inner Group")
    set(RunCMake_TEST_FAILED "Invalid Inner Group path: '${inner_group_path}'" PARENT_SCOPE)
    return()
  endif()
  MapFind(PROJECT_PARENTS "${inner_group_guid}" inner_group_parent_guid)
  if(NOT DEFINED inner_group_parent_guid)
    set(RunCMake_TEST_FAILED "Inner Group is not nested" PARENT_SCOPE)
    return()
  endif()
  if(NOT "${inner_group_parent_guid}" STREQUAL "${outer_group_guid}")
    set(RunCMake_TEST_FAILED "Inner Group is not nested within Outer Group" PARENT_SCOPE)
    return()
  endif()


  # Check solution items and nesting:
  MapLength(SOLUTION_ITEMS solution_item_count)
  if(NOT "${solution_item_count}" EQUAL 4)
    set(RunCMake_TEST_FAILED "Unexpected number of solution items: ${solution_item_count}")
    return()
  endif()

  MapFind(SOLUTION_ITEMS "solution-item-0-1.txt" group_name)
  if(NOT DEFINED group_name)
    set(RunCMake_TEST_FAILED "Solution item not found: solution-item-0-1.txt")
    return()
  endif()
  if(NOT "${group_name}" STREQUAL "Solution Items")
    set(RunCMake_TEST_FAILED "Invalid group for solution-item-0-1.txt: '${group_name}'")
    return()
  endif()

  MapFind(SOLUTION_ITEMS "solution-item-1-1.txt" group_name)
  if(NOT DEFINED group_name)
    set(RunCMake_TEST_FAILED "Solution item not found: solution-item-1-1.txt")
    return()
  endif()
  if(NOT "${group_name}" STREQUAL "Outer Group")
    set(RunCMake_TEST_FAILED "Invalid group for solution-item-1-1.txt: '${group_name}'")
    return()
  endif()

  MapFind(SOLUTION_ITEMS "solution-item-2-1.txt" group_name)
  if(NOT DEFINED group_name)
    set(RunCMake_TEST_FAILED "Solution item not found: solution-item-2-1.txt")
    return()
  endif()
  if(NOT "${group_name}" STREQUAL "Inner Group")
    set(RunCMake_TEST_FAILED "Invalid group for solution-item-2-1.txt: '${group_name}'")
    return()
  endif()

  MapFind(SOLUTION_ITEMS "solution-item-2-2.txt" group_name)
  if(NOT DEFINED group_name)
    set(RunCMake_TEST_FAILED "Solution item not found: solution-item-2-2.txt")
    return()
  endif()
  if(NOT "${group_name}" STREQUAL "Inner Group")
    set(RunCMake_TEST_FAILED "Invalid group for solution-item-2-2.txt: '${group_name}'")
    return()
  endif()
endblock()


# Check the nested solution:
block()
  ParseSln("${RunCMake_TEST_BINARY_DIR}/SolutionItems/SolutionItemsSubproject.sln")

  if(DEFINED RunCMake_TEST_FAILED)
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return()
  endif()


  # Check group guids and nesting:

  MapFind(GROUP_GUIDS "Extraneous" root_group_guid)
  if(NOT DEFINED root_group_guid)
    set(RunCMake_TEST_FAILED "Extraneous not found" PARENT_SCOPE)
    return()
  endif()
  MapFind(GROUP_PATHS "Extraneous" root_group_path)
  if(NOT "${root_group_path}" STREQUAL "Extraneous")
    set(RunCMake_TEST_FAILED "Invalid Extraneous path: '${root_group_path}'" PARENT_SCOPE)
    return()
  endif()
  MapFind(PROJECT_PARENTS "${root_group_guid}" root_group_parent_guid)
  if(DEFINED root_group_parent_guid)
    set(RunCMake_TEST_FAILED "Extraneous is nested" PARENT_SCOPE)
    return()
  endif()


  # Check solution items and nesting:

  MapLength(SOLUTION_ITEMS solution_item_count)
  if(NOT "${solution_item_count}" EQUAL 1)
    set(RunCMake_TEST_FAILED "Unexpected number of solution items: ${solution_item_count}" PARENT_SCOPE)
    return()
  endif()

  MapFind(SOLUTION_ITEMS "extraneous.txt" group_name)
  if(NOT DEFINED group_name)
    set(RunCMake_TEST_FAILED "Solution item not found: extraneous.txt" PARENT_SCOPE)
    return()
  endif()
  if(NOT "${group_name}" STREQUAL "Extraneous")
    set(RunCMake_TEST_FAILED "Invalid group for extraneous.txt: '${group_name}'" PARENT_SCOPE)
    return()
  endif()
endblock()
