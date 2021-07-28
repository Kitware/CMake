function (check_directory_property dir prop)
  get_directory_property(gdp_val DIRECTORY "${dir}" "${prop}")
  get_property(gp_val
    DIRECTORY "${dir}"
    PROPERTY "${prop}")

  message("get_directory_property: -->${gdp_val}<--")
  message("get_property: -->${gp_val}<--")
endfunction ()

set_directory_properties(PROPERTIES empty "" custom value)

check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" empty)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" custom)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" noexist)

add_custom_target(CustomTop)
add_library(InterfaceTop INTERFACE)
add_library(my::InterfaceTop ALIAS InterfaceTop)

add_library(Imported1Top INTERFACE IMPORTED)
add_library(Imported2Top INTERFACE IMPORTED)

add_subdirectory(directory_properties)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" SUBDIRECTORIES)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" SUBDIRECTORIES)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" BUILDSYSTEM_TARGETS)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" BUILDSYSTEM_TARGETS)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" IMPORTED_TARGETS)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" IMPORTED_TARGETS)

check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" BINARY_DIR)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" SOURCE_DIR)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" BINARY_DIR)
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" SOURCE_DIR)

check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" TESTS)
add_test(NAME test1 COMMAND "${CMAKE_COMMAND}" -E echo "test1")
add_test(NAME test2 COMMAND "${CMAKE_COMMAND}" -E echo "test2")
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" TESTS)
add_test(NAME test3 COMMAND "${CMAKE_COMMAND}" -E echo "test3")
check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}" TESTS)

check_directory_property("${CMAKE_CURRENT_SOURCE_DIR}/directory_properties" TESTS)
