add_executable(main main.cpp)
target_compile_definitions(main PRIVATE EXPECT_${CMP0231_EXPECT})
# MSVC does not support modules in non-permissive mode.
set_property(TARGET main PROPERTY CXX_SCAN_FOR_MODULES OFF)
if(CMP0231_STANDARD)
  set_property(TARGET main PROPERTY CXX_STANDARD ${CMP0231_STANDARD})
endif()
set_property(TARGET main PROPERTY CXX_EXTENSIONS ${CMP0231_EXTENSIONS})

# The policy is recorded when a target is created.  Changing it afterwards
# affects only the targets created later.
if(CMP0231_LATE_POLICY)
  cmake_policy(SET CMP0231 ${CMP0231_LATE_POLICY})
  add_executable(late main.cpp)
  target_compile_definitions(late PRIVATE EXPECT_${CMP0231_LATE_EXPECT})
  set_property(TARGET late PROPERTY CXX_SCAN_FOR_MODULES OFF)
  if(CMP0231_STANDARD)
    set_property(TARGET late PROPERTY CXX_STANDARD ${CMP0231_STANDARD})
  endif()
  set_property(TARGET late PROPERTY CXX_EXTENSIONS ${CMP0231_EXTENSIONS})
  # Restore the policy for the try_compile checks below.
  cmake_policy(SET CMP0231 ${CMP0231_POLICY})
endif()

# The policy setting is propagated to try_compile projects, which build in
# the same mode as the main target.  The source fails to compile in the
# other mode.
if(CMP0231_STANDARD)
  set(CMAKE_CXX_STANDARD ${CMP0231_STANDARD})
endif()
set(CMAKE_CXX_EXTENSIONS ${CMP0231_EXTENSIONS})
if(CMP0231_EXPECT STREQUAL "PERMISSIVE")
  set(other_mode NON_PERMISSIVE)
else()
  set(other_mode PERMISSIVE)
endif()
try_compile(expected_mode_ok
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  COMPILE_DEFINITIONS -DEXPECT_${CMP0231_EXPECT}
  CMAKE_FLAGS -DCMAKE_CXX_SCAN_FOR_MODULES=OFF
  NO_CACHE
  )
if(NOT expected_mode_ok)
  message(SEND_ERROR "try_compile did not build in ${CMP0231_EXPECT} mode")
endif()
try_compile(other_mode_ok
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  COMPILE_DEFINITIONS -DEXPECT_${other_mode}
  CMAKE_FLAGS -DCMAKE_CXX_SCAN_FOR_MODULES=OFF
  NO_CACHE
  )
if(other_mode_ok)
  message(SEND_ERROR "try_compile built in ${other_mode} mode unexpectedly")
endif()
