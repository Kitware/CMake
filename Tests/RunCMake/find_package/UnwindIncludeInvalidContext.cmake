cmake_policy(SET CMP0074 NEW)
set(PrimaryUnwind_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/UnwindInclude)

set(UNWIND_TARGET NoUnwind)
find_package(PrimaryUnwind UNWIND_INCLUDE)
