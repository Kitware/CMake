INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_LIBRARY( Two TwoSrc.c )
TARGET_LINK_LIBRARIES( Two Three )

# Setup a target to cause failure if Two does not depend on it or if
# Two actually links to it.  This will test that a utility dependency
# on a library target works properly.
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/two-test.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_CURRENT_SOURCE_DIR}/two-test.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/two-test.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/two-test.h.in
  )
ADD_LIBRARY( TwoCustom TwoCustomSrc.c ${CMAKE_CURRENT_BINARY_DIR}/two-test.h)
SET_TARGET_PROPERTIES(TwoCustom PROPERTIES EXCLUDE_FROM_ALL 1)
TARGET_LINK_LIBRARIES(TwoCustom Three)

# Add a utility dependency to make sure it works without linking.
ADD_DEPENDENCIES(Two TwoCustom)
