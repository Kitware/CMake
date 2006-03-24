PROJECT(Object)
IF(APPLE)
  # set _CMAKE_OSX_MACHINE to umame -m
  EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)
  # check for Power PC and change to ppc
  IF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
    SET(_CMAKE_OSX_MACHINE ppc)
  ENDIF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
  SET(CMAKE_OSX_ARCHITECTURES ${_CMAKE_OSX_MACHINE})
ENDIF(APPLE)

ADD_EXECUTABLE(external external_object.cxx external_main.cxx)
