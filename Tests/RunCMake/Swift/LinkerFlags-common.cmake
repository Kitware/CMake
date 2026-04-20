enable_language(Swift)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -foo")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -foo")

add_library(L SHARED L.swift)
add_executable(E E.swift)
target_link_libraries(E PRIVATE L)
