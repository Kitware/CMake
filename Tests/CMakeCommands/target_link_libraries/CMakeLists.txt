cmake_minimum_required(VERSION 2.8)

project(target_link_libraries)

file(WRITE
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
  "int main() { return 0; }
"
)

add_executable(
  target_link_libraries
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
)


set (COUNT 0)
macro(_do_build CLEAR_LINK_INTERFACE_LIBRARIES SPECIFY_LINK_INTERFACE_LIBRARIES)
  math(EXPR COUNT "${COUNT} + 1" )

  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}")

  if (${CLEAR_LINK_INTERFACE_LIBRARIES})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
      "set(CMAKE_LINK_INTERFACE_LIBRARIES \"\")\n"
    )
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp"
    ""
  )

  foreach(klass ${ARGN})
    if (klass STREQUAL "LIBS")
      break()
    endif()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp"
      "#include \"${klass}.h\"\n"
    )
  endforeach()

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp"
    "int main(int, char **) {"
  )

  foreach(klass ${ARGN})
    if (klass STREQUAL "LIBS")
      break()
    endif()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp"
      "${klass} ${klass}_;\n"
    )
  endforeach()

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp"
    "return 0;"
    "}\n"
  )

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"

    "add_library(libA SHARED classA.cpp)\n"
    "add_library(libB SHARED classB.cpp)\n"
    "add_library(libC SHARED classC.cpp)\n"

    "generate_export_header(libA)\n"
    "generate_export_header(libB)\n"
    "generate_export_header(libC)\n"

    "add_executable(exec \"${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp\")\n"
  )

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    "target_link_libraries(libB libA)\n"
    "target_link_libraries(libC libA libB)\n"
  )

  if (${SPECIFY_LINK_INTERFACE_LIBRARIES})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
      "target_link_libraries(libB LINK_INTERFACE_LIBRARIES libA)\n"
      "target_link_libraries(libC LINK_INTERFACE_LIBRARIES libA)\n"
    )
  endif()

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    "target_link_libraries(exec "
  )

  set(continue True)
  foreach(klass ${ARGN})
    if (klass STREQUAL "LIBS")
      # CMake has no continue() command
      set(continue False)
    endif()
    if (NOT continue AND NOT klass STREQUAL "LIBS")
      file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
        "${klass} "
      )
    endif()
  endforeach()

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    ")\n"
  )

  try_compile(Result ${CMAKE_CURRENT_BINARY_DIR}/fail${COUNT}
    ${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}
    src
    OUTPUT_VARIABLE Out
  )
endmacro()

macro(EXPECT_PASS)
  _do_build(${ARGN})
  if (NOT ${Result})
    message(SEND_ERROR "Failed! \n\n${Out}")
  endif()
endmacro()

macro(EXPECT_FAIL)
  _do_build(${ARGN})
  if (${Result})
    message(SEND_ERROR "Failed! \n\n${Out} -- ${COUNT}")
  endif()
endmacro()

macro(all_pass)
  expect_pass(False False ${ARGN})
  expect_pass(True False ${ARGN})
  expect_pass(False True ${ARGN})
  expect_pass(True True ${ARGN})
endmacro()

# If we specify all libs to the executable, it will always pass
all_pass(
    classA
  LIBS
    libA
)

all_pass(
    classA classB
  LIBS
    libA libB
)

all_pass(
    classA classC
  LIBS
    libA libC
)

all_pass(
    classA classB classC
  LIBS
    libA libB libC
)

# If we don't set the CMAKE_LINK_INTERFACE_LIBRARIES to empty and we don't explicitly
# specify the LINK_INTERFACE_LIBRARIES, the executable can use all classes if linked
# indirectly to them
expect_pass(False False
    classA classB
  LIBS
    libB
)

# Although libC is linked to libA, libA is not part of the link interface of libC.
# Because we don't clear the CMAKE_LINK_INTERFACE_LIBRARIES it still works.
expect_pass(False False
    classA classC
  LIBS
    libC
)

# However, if we do clear it and don't explicitly link the executable to it, it fails,
# whether we specify the link_interface_libraries properly or not.
if (NOT APPLE)
  # Apparently this doesn't work on APPLE either.
  # http://www.cdash.org/CDash/testDetails.php?test=117927753&build=1596834
  if (NOT WIN32)
    # This does not work on windows apparently
    expect_fail(True False
      classB classC
    LIBS
      libC
  )
  endif()

  expect_fail(True True
      classB classC
    LIBS
      libC
  )
endif()

# Then we can still link the executable to libA directly of course to pass
expect_pass(True False
    classA classC
  LIBS
    libC libA
)
expect_pass(True True
    classA classC
  LIBS
    libC libA
)

# Also, if we clear and specify the link interface, we can use classes defined in
# that interface without linking to the library directly
expect_pass(True True
    classA classB
  LIBS
    libB
)

expect_pass(True True
    classA classC
  LIBS
    libC
)
