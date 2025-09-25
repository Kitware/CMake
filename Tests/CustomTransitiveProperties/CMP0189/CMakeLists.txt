cmake_policy(SET CMP0189 NEW)
set(out "${CMAKE_CURRENT_BINARY_DIR}/out-$<CONFIG>.txt")
file(GENERATE OUTPUT "${out}" CONTENT "# file(GENERATE) produced:
${in_LINK_LIBRARIES}
")
add_custom_target(check-CMP0189-NEW ALL VERBATIM
  COMMAND ${CMAKE_COMMAND} -Dconfig=$<CONFIG> -Dout=${out} -P${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  COMMAND check-args
  "$<TARGET_PROPERTY:iface1,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface1,INTERFACE_LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface2,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface2,INTERFACE_LINK_LIBRARIES>" "iface1"
  "$<TARGET_PROPERTY:static1,LINK_LIBRARIES>" "iface2;iface1"
  "$<TARGET_PROPERTY:static1,INTERFACE_LINK_LIBRARIES>" "iface2;iface1"
  "$<TARGET_PROPERTY:CustomTransitiveProperties,LINK_LIBRARIES>" "static1;object1;iface2;iface1;iface2"
  "$<TARGET_PROPERTY:CustomTransitiveProperties,INTERFACE_LINK_LIBRARIES>" ""
  COMMAND check-args
  "$<TARGET_PROPERTY:iface10,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface10,INTERFACE_LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface11,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface11,INTERFACE_LINK_LIBRARIES>" "iface10"
  "$<TARGET_PROPERTY:static10,LINK_LIBRARIES>" "iface11;iface10"
  #                                              _/         \__
  #                                             /              \
  #                               "static10[iface11];iface11[iface10]"
  "$<TARGET_PROPERTY:static10,INTERFACE_LINK_LIBRARIES>" "iface11;iface10"
  "$<TARGET_PROPERTY:static11,LINK_LIBRARIES>" "static10;iface11;iface11;iface10"
  #                                             __/      __/         \__     \__________
  #                                            /        /               \               \
  #                              "static11[static10;iface11];static10[iface11;iface11[iface10]]"
  "$<TARGET_PROPERTY:static11,INTERFACE_LINK_LIBRARIES>" "static10;iface11;iface11;iface10"
  "$<TARGET_PROPERTY:main10,LINK_LIBRARIES>" "static11;static10;static10;iface11;iface11;iface10"
  #                                      _______/ _______/         |        |        \______ \______________
  #                                     /        /                 |        |               \               \
  #                         "main10[static11;static10];static11[static10;iface11;static10[iface11;iface11[iface10]]]"
  "$<TARGET_PROPERTY:main10,INTERFACE_LINK_LIBRARIES>" ""
  COMMAND check-args
  "$<TARGET_PROPERTY:iface20,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface20,INTERFACE_LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface21,LINK_LIBRARIES>" ""
  "$<TARGET_PROPERTY:iface21,INTERFACE_LINK_LIBRARIES>" "iface20"
  "$<TARGET_PROPERTY:static20,LINK_LIBRARIES>" "iface21;iface20"
  #                                              _/         \__
  #                                             /              \
  #                               "static20[iface21];iface21[iface20]"
  "$<TARGET_PROPERTY:static20,INTERFACE_LINK_LIBRARIES>" "iface21;iface20"
  "$<TARGET_PROPERTY:static21,LINK_LIBRARIES>" "static20;iface21;iface21;iface20"
  #                                             __/      __/         \__     \__________
  #                                            /        /               \               \
  #                              "static21[static20;iface21];static20[iface21;iface21[iface20]]"
  "$<TARGET_PROPERTY:static21,INTERFACE_LINK_LIBRARIES>" "static20;iface21;iface21;iface20"
  "$<TARGET_PROPERTY:main20,LINK_LIBRARIES>" "static21;static20;static20;iface21;iface21;iface20"
  #                                      _______/ _______/         |        |        \______ \______________
  #                                     /        /                 |        |               \               \
  #                         "main20[static21;static20];static21[static20;iface21;static20[iface21;iface21[iface20]]]"
  "$<TARGET_PROPERTY:main20,INTERFACE_LINK_LIBRARIES>" ""
  )
