add_executable(main main.cpp)
target_compile_definitions(main PRIVATE EXPECT_${CMP0232_EXPECT})
if(CMP0232_STANDARD)
  set_property(TARGET main PROPERTY CXX_STANDARD ${CMP0232_STANDARD})
endif()
if(CMP0232_OPT_OUT)
  target_compile_options(main PRIVATE -Zc:__cplusplus-)
endif()

# The policy setting is propagated to try_compile projects.
try_compile(cplusplus_updated
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  COMPILE_DEFINITIONS -DEXPECT_UPDATED
  NO_CACHE
  )
if(CMP0232_TRY_COMPILE_EXPECT STREQUAL "UPDATED" AND NOT cplusplus_updated)
  message(SEND_ERROR "try_compile did not update __cplusplus")
elseif(CMP0232_TRY_COMPILE_EXPECT STREQUAL "LEGACY" AND cplusplus_updated)
  message(SEND_ERROR "try_compile updated __cplusplus unexpectedly")
endif()
