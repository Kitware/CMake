install(FILES CMakeLists.txt DESTINATION foo COMPONENT foo)
install(FILES CMakeLists.txt DESTINATION bar COMPONENT bar)
install(FILES CMakeLists.txt DESTINATION bas COMPONENT bas)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/preinst "echo default_preinst")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/prerm "echo default_prerm")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_BINARY_DIR}/preinst;${CMAKE_CURRENT_BINARY_DIR}/prerm")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bar/preinst "echo bar_preinst")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bar/prerm "echo bar_prerm")

set(CPACK_DEBIAN_BAR_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_BINARY_DIR}/bar/preinst;${CMAKE_CURRENT_BINARY_DIR}/bar/prerm")

set(CPACK_PACKAGE_NAME "deb_extra")
set(CPACK_PACKAGE_CONTACT "someone")

set(CPACK_DEB_COMPONENT_INSTALL ON)
