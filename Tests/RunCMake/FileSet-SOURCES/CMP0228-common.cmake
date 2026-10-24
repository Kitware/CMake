enable_language(C)

# The policy setting of the file set owner applies.
if (DEFINED CMP0228_OWNER)
  cmake_policy(SET CMP0228 ${CMP0228_OWNER})
endif()
add_subdirectory(FileSetInterfaceConsumer)

# The policy setting of the consumers must not matter.
if (CMP0228_OWNER STREQUAL "NEW")
  cmake_policy(SET CMP0228 OLD)
else()
  cmake_policy(SET CMP0228 NEW)
endif()

foreach (id IN ITEMS 1 2)
  add_executable(consumer${id})
  set_property(TARGET consumer${id} PROPERTY FS_ID ${id})
  if (CMP0228_OWNER STREQUAL "NEW")
    # Each consumer compiles the file set with the requirements evaluated for
    # it.
    target_compile_definitions(consumer${id} PRIVATE EXPECTED_ID=${id})
  else()
    # The requirements are evaluated for the owner of the file set.
    target_compile_definitions(consumer${id} PRIVATE EXPECTED_ID=0)
  endif()
  target_link_libraries(consumer${id} PRIVATE iface)
endforeach()
