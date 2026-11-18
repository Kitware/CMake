set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(main main.c ${CMP0233_EXTRA_SOURCES})
target_compile_definitions(main PRIVATE EXPECT_${CMP0233_EXPECT})
set_property(TARGET main PROPERTY C_STANDARD ${CMP0233_STANDARD})
set_property(TARGET main PROPERTY C_EXTENSIONS ${CMP0233_EXTENSIONS})

# The policy is recorded when a target is created.  Changing it afterwards
# affects only the targets created later.  This is not done in a function,
# which would use the policy settings from where it is defined.
if(CMP0233_LATE_POLICY)
  cmake_policy(SET CMP0233 ${CMP0233_LATE_POLICY})
  add_executable(late main.c)
  target_compile_definitions(late PRIVATE EXPECT_${CMP0233_LATE_EXPECT})
  set_property(TARGET late PROPERTY C_STANDARD ${CMP0233_STANDARD})
  set_property(TARGET late PROPERTY C_EXTENSIONS ${CMP0233_EXTENSIONS})
  # Restore the policy for the try_compile checks below.
  cmake_policy(SET CMP0233 ${CMP0233_POLICY})
endif()

# The policy setting is propagated to try_compile projects, which build in
# the same mode as the main target.  The source fails to compile in the
# other mode.
set(CMAKE_C_STANDARD ${CMP0233_STANDARD})
set(CMAKE_C_EXTENSIONS ${CMP0233_EXTENSIONS})
if(CMP0233_EXPECT STREQUAL "STRICT")
  set(other_mode EXTENSIONS)
else()
  set(other_mode STRICT)
endif()
try_compile(expected_mode_ok
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  COMPILE_DEFINITIONS -DEXPECT_${CMP0233_EXPECT}
  NO_CACHE
  )
if(NOT expected_mode_ok)
  message(SEND_ERROR "try_compile did not build in ${CMP0233_EXPECT} mode")
endif()
try_compile(other_mode_ok
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  COMPILE_DEFINITIONS -DEXPECT_${other_mode}
  NO_CACHE
  )
if(other_mode_ok)
  message(SEND_ERROR "try_compile built in ${other_mode} mode unexpectedly")
endif()
