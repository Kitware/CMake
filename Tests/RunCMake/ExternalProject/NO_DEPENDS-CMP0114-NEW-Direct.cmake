cmake_policy(SET CMP0114 NEW)
include(ExternalProject)
ExternalProject_Add(BAR SOURCE_DIR .  TEST_COMMAND echo test)
ExternalProject_Add_StepTargets(BAR NO_DEPENDS test)
