install(FILES CMakeLists.txt DESTINATION foo1 COMPONENT comp1.test1)
install(FILES CMakeLists.txt DESTINATION foo2 COMPONENT comp2::test2)
install(FILES CMakeLists.txt DESTINATION foo3 COMPONENT comp3/test3)

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  foreach(gen IN ITEMS ARCHIVE DEBIAN RPM)
    set(CPACK_${gen}_COMP2::TEST2_FILE_NAME "component_with_special_chars-0.1.1-${CMAKE_SYSTEM_NAME}-component2")
    set(CPACK_${gen}_COMP3/TEST3_FILE_NAME  "component_with_special_chars-0.1.1-${CMAKE_SYSTEM_NAME}-component3")
  endforeach()
elseif(PACKAGING_TYPE STREQUAL "GROUP")
  set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
  foreach(gen IN ITEMS ARCHIVE DEB RPM)
    set(CPACK_${gen}_COMPONENT_INSTALL ON)
  endforeach()
  include(CPackComponent)

  cpack_add_component_group(group1 DISPLAY_NAME "Group 1")
  cpack_add_component_group(group2 DISPLAY_NAME "Group 2")
  cpack_add_component(comp1.test1
          DISPLAY_NAME "Group 1"
          DESCRIPTION "Component for Group 1"
          GROUP group1
  )
  cpack_add_component(comp2::test2
          DISPLAY_NAME "Group 1"
          DESCRIPTION "Component for Group 1"
          GROUP group1
  )
  cpack_add_component(comp3/test3
          DISPLAY_NAME "Group 2"
          DESCRIPTION "Component for Group 2"
          GROUP group2
  )
endif()
