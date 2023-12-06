enable_language(C)

add_library(Stub SHARED Stub.c)
set_target_properties(Stub PROPERTIES
  SOVERSION 1
  LIBRARY_OUTPUT_DIRECTORY lib
  )

set(StubDir ${CMAKE_CURRENT_BINARY_DIR}/lib/stubs)
set(Stub "${StubDir}/${CMAKE_SHARED_LIBRARY_PREFIX}Stub${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_custom_target(StubCopy
  COMMAND ${CMAKE_COMMAND} -E make_directory "${StubDir}"
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_SONAME_FILE:Stub>" "${Stub}"
  BYPRODUCTS ${Stub}
  )
add_dependencies(StubCopy Stub)
add_library(Imp::Stub SHARED IMPORTED)
set_property(TARGET Imp::Stub PROPERTY IMPORTED_IMPLIB "${Stub}")
add_dependencies(Imp::Stub StubCopy)

add_library(StubUse SHARED StubUse.c)
target_link_libraries(StubUse PRIVATE Imp::Stub)

add_executable(StubExe StubExe.c)
target_link_libraries(StubExe PRIVATE StubUse)
