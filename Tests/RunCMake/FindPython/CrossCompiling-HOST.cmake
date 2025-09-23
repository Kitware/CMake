cmake_policy(SET CMP0190 NEW)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "${CMAKE_COMMAND}" -P raise-error.cmake)

enable_language(C)

find_package(${PYTHON} ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter)
