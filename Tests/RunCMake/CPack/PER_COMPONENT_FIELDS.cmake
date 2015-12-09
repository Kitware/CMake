install(FILES CMakeLists.txt DESTINATION foo COMPONENT pkg_1)
install(FILES CMakeLists.txt DESTINATION foo COMPONENT pkg_2)
install(FILES CMakeLists.txt DESTINATION foo COMPONENT pkg_3)

set(CPACK_PACKAGE_NAME "per_component")
