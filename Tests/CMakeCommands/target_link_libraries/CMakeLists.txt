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

macro(ASSERT_PROPERTY _target _property _value)
  get_target_property(_out ${_target} ${_property})
  if (NOT _out)
    set(_out "")
  endif()
  if (NOT "${_out}" STREQUAL "${_value}")
    message(SEND_ERROR "Target ${_target} does not have property ${_property} with value ${_value}. Actual value: ${_out}")
  endif()
endmacro()

include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(depA SHARED depA.cpp)
generate_export_header(depA)

add_library(depB SHARED depB.cpp)
generate_export_header(depB)

target_link_libraries(depB LINK_PRIVATE depA)

add_library(depC SHARED depC.cpp)
generate_export_header(depC)

target_link_libraries(depC LINK_PUBLIC depA)

assert_property(depA LINK_INTERFACE_LIBRARIES "")
assert_property(depB LINK_INTERFACE_LIBRARIES "")
assert_property(depC LINK_INTERFACE_LIBRARIES "depA")

add_executable(targetA targetA.cpp)

target_link_libraries(targetA LINK_INTERFACE_LIBRARIES depA depB)

assert_property(targetA LINK_INTERFACE_LIBRARIES "depA;depB")

set_target_properties(targetA PROPERTIES LINK_INTERFACE_LIBRARIES "")

assert_property(targetA LINK_INTERFACE_LIBRARIES "")

target_link_libraries(targetA depB depC)

assert_property(targetA LINK_INTERFACE_LIBRARIES "")

set(Bools True False)

foreach(B1 ${Bools})
  foreach(B2 ${Bools})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/libs"
      "${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_${B2}"
    )
  endforeach()
endforeach()

foreach(B1 ${Bools})
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/libs_True_${B1}/CMakeLists.txt"
    "set(CMAKE_LINK_INTERFACE_LIBRARIES \"\")\n"
  )
endforeach()

foreach(B1 ${Bools})
  foreach(B2 ${Bools})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_${B2}/CMakeLists.txt"

      "add_library(libA SHARED classA.cpp)\n"
      "add_library(libB SHARED classB.cpp)\n"
      "add_library(libC SHARED classC.cpp)\n"

      "generate_export_header(libA)\n"
      "generate_export_header(libB)\n"
      "generate_export_header(libC)\n"

      "target_link_libraries(libB libA)\n"
      "target_link_libraries(libC libA libB)\n"
    )
  endforeach()
endforeach()

foreach(B1 ${Bools})
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_True/CMakeLists.txt"
      "target_link_libraries(libB LINK_INTERFACE_LIBRARIES libA)\n"
      "target_link_libraries(libC LINK_INTERFACE_LIBRARIES libA)\n"
    )
endforeach()

foreach(B1 ${Bools})
  foreach(B2 ${Bools})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_${B2}/CMakeLists.txt"
      "export(TARGETS libA libB libC FILE Targets.cmake)\n"
    )

    try_compile(Result "${CMAKE_CURRENT_BINARY_DIR}/libs_build_${B1}_${B2}"
      "${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_${B2}"
      libs
      OUTPUT_VARIABLE Out
    )
    if (NOT Result)
      message(SEND_ERROR "Could not build shared library
              Libs CLEAR_LINK_INTERFACE_LIBRARIES: ${B1}
              Libs SPECIFY_LINK_INTERFACE_LIBRARIES: ${B2}\n\n\n${Out}")
    endif()
  endforeach()
endforeach()


set (COUNT 0)
macro(_do_build CLEAR_LINK_INTERFACE_LIBRARIES
                SPECIFY_LINK_INTERFACE_LIBRARIES)
  math(EXPR COUNT "${COUNT} + 1" )

  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}"
  )

  # Set local short variables to avoid long lines
  set(B1 ${CLEAR_LINK_INTERFACE_LIBRARIES})
  set(B2 ${SPECIFY_LINK_INTERFACE_LIBRARIES})
  set(Libs_Build_Dir "${CMAKE_CURRENT_BINARY_DIR}/libs_build_${B1}_${B2}")

  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    "include_directories(\"${CMAKE_CURRENT_BINARY_DIR}/libs_${B1}_${B2}\"\n"
    "                    \"${Libs_Build_Dir}\"\n"
    ")\n"
    "include(\"${Libs_Build_Dir}/Targets.cmake\")\n"
  )

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

    "add_executable(exec \n"
    "\"${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/main.cpp\")\n"
  )

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

  try_compile(Result "${CMAKE_CURRENT_BINARY_DIR}/fail${COUNT}"
    "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}"
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
    message(SEND_ERROR "Failed! \n\n${Out}")
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

# If we don't set the CMAKE_LINK_INTERFACE_LIBRARIES to empty and
# we don't explicitly specify the LINK_INTERFACE_LIBRARIES, the
# executable can use all classes if linked
# indirectly to them
expect_pass(False False
    classA classB
  LIBS
    libB
)

# Although libC is linked to libA, libA is not part of the link interface of
# libC.
# Because we don't clear the CMAKE_LINK_INTERFACE_LIBRARIES it still works.
expect_pass(False False
    classA classC
  LIBS
    libC
)

# However, if we do clear it and don't explicitly link the executable to it,
# it fails,
# whether we specify the link_interface_libraries properly or not.
if (NOT APPLE)
  expect_fail(True False
    classB classC
  LIBS
    libC
  )
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

# Also, if we clear and specify the link interface, we can use classes defined
# in that interface without linking to the library directly
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
