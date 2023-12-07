include(ExternalProject)
ExternalProject_Add(Foo
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Foo
  BUILD_COMMAND ${DETECT_JOBSERVER} "ep.txt"
  BUILD_JOB_SERVER_AWARE 1
  INSTALL_COMMAND ""
)

# Add a second step to test JOB_SERVER_AWARE
ExternalProject_Add_Step(Foo
  second_step
  COMMAND ${DETECT_JOBSERVER} "ep_second_step.txt"
  DEPENDEES build
  ALWAYS 1
  JOB_SERVER_AWARE 1
)
