enable_language(C)

set(CMAKE_AIX_SHARED_LIBRARY_ARCHIVE 1)

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  set(suffix "${CMAKE_SHARED_LIBRARY_ARCHIVE_SUFFIX}")
  set(genex "TARGET_LINKER_FILE")
else()
  set(suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(genex "TARGET_SONAME_FILE")
endif()

add_library(Stub SHARED Stub.c)
set_target_properties(Stub PROPERTIES
  SOVERSION 1
  LIBRARY_OUTPUT_DIRECTORY lib
  )

set(StubDir ${CMAKE_CURRENT_BINARY_DIR}/lib/stubs)
set(Stub "${StubDir}/${CMAKE_SHARED_LIBRARY_PREFIX}Stub${suffix}")
add_custom_target(StubCopy
  COMMAND ${CMAKE_COMMAND} -E make_directory "${StubDir}"
  COMMAND ${CMAKE_COMMAND} -E copy "$<${genex}:Stub>" "${Stub}"
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
