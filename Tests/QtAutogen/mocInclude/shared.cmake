# Test moc include patterns
include_directories("../mocInclude")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate .moc file externally and enabled SKIP_AUTOMOC on the file
qtx_generate_moc(
  ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjA.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/SObjA.moc)
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjA.cpp PROPERTY SKIP_AUTOMOC ON)

# Generate .moc file externally from generated source file
# and enabled SKIP_AUTOMOC on the source file
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SObjB.hpp
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjB.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/SObjB.hpp)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SObjB.cpp
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjB.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/SObjB.cpp)
qtx_generate_moc(
  ${CMAKE_CURRENT_BINARY_DIR}/SObjB.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/SObjB.moc)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/SObjB.cpp PROPERTY SKIP_AUTOMOC ON)

# Generate moc file externally and enabled SKIP_AUTOMOC on the header
qtx_generate_moc(
  ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjCExtra.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/SObjCExtra_extMoc.cpp)
set_property(
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjCExtra.hpp
  PROPERTY SKIP_AUTOMOC ON)
# Custom target to depend on
set(SOBJC_MOC ${CMAKE_CURRENT_BINARY_DIR}/moc_SObjCExtra.cpp)
add_custom_target("${MOC_INCLUDE_NAME}_SOBJC"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SObjCExtra_extMoc.cpp
  BYPRODUCTS ${SOBJC_MOC}
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocInclude/SObjCExtra.moc.in
    ${SOBJC_MOC})

# MOC_INCLUDE_NAME must be defined by the includer
add_executable(${MOC_INCLUDE_NAME}
  # Common sources
  ../mocInclude/ObjA.cpp
  ../mocInclude/ObjB.cpp

  ../mocInclude/LObjA.cpp
  ../mocInclude/LObjB.cpp

  ../mocInclude/EObjA.cpp
  ../mocInclude/EObjAExtra.cpp
  ../mocInclude/EObjB.cpp
  ../mocInclude/subExtra/EObjBExtra.cpp

  ../mocInclude/SObjA.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/SObjA.moc
  ${CMAKE_CURRENT_BINARY_DIR}/SObjB.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/SObjB.moc
  ../mocInclude/SObjC.cpp
  ../mocInclude/SObjCExtra.hpp
  ../mocInclude/SObjCExtra.cpp

  ../mocInclude/subGlobal/GObj.cpp
  main.cpp
)
add_dependencies(${MOC_INCLUDE_NAME} "${MOC_INCLUDE_NAME}_SOBJC")
target_link_libraries(${MOC_INCLUDE_NAME} ${QT_LIBRARIES})
set_target_properties(${MOC_INCLUDE_NAME} PROPERTIES AUTOMOC ON)
