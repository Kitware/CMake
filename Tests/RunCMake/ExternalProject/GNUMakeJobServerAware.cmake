include(ExternalProject)
ExternalProject_Add(Foo
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Foo
  BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
  BUILD_JOB_SERVER_AWARE 1
  INSTALL_COMMAND ""
)

# Add a second step to test JOB_SERVER_AWARE
ExternalProject_Add_Step(Foo
  second_step
  COMMAND ${CMAKE_COMMAND} -E true
  DEPENDEES build
  ALWAYS 1
  JOB_SERVER_AWARE 1
)
