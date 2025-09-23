cmake_policy(SET CMP0190 NEW)

enable_language(C)


# Search for host Interpreter
set(${PYTHON}_ARTIFACTS_PREFIX "_HOST")

find_package(${PYTHON} ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter)

unset(${PYTHON}_ARTIFACTS_PREFIX)


# Search cross-compilation artifacts
## First, built an pseudo-emulator
set(PSEUDO_EMULATOR_DIR "${CMAKE_CURRENT_BINARY_DIR}/pseudo_emulator")

file(MAKE_DIRECTORY "${PSEUDO_EMULATOR_DIR}")

execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -DCMAKE_BUILD_TYPE=Release
                                           -S "${CMAKE_CURRENT_SOURCE_DIR}/pseudo_emulator"
                                           -B "${PSEUDO_EMULATOR_DIR}"
                COMMAND_ERROR_IS_FATAL ANY)

execute_process(COMMAND "${CMAKE_COMMAND}" --build "${PSEUDO_EMULATOR_DIR}"
                COMMAND_ERROR_IS_FATAL ANY)

## Now, configure this pseudo-emulator
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "${PSEUDO_EMULATOR_DIR}/pseudo_emulator")


find_package(${PYTHON} ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter Development)
