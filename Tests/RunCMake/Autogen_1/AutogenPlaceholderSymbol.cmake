enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

if(APPLE)
  # Archive with libtool, which warns about members without global symbols.
  get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
  find_program(LIBTOOL libtool HINTS "${compiler_dir}" /usr/bin REQUIRED)
  set(CMAKE_CXX_CREATE_STATIC_LIBRARY
    "\"${LIBTOOL}\" -static -o <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

set(CMAKE_AUTOMOC ON)

add_library(NoMocStatic STATIC AutogenPlaceholderSymbol.cxx)
add_library(NoMocObject OBJECT AutogenPlaceholderSymbol.cxx)
add_library(NoMocShared SHARED AutogenPlaceholderSymbol.cxx)
add_executable(NoMocExe AutogenPlaceholderSymbol.cxx)
target_compile_definitions(NoMocExe PRIVATE WITH_MAIN)

# Archive the placeholder objects of non-static targets.
add_library(FromObjects STATIC $<TARGET_OBJECTS:NoMocObject>)
add_library(FromSharedObjects STATIC $<TARGET_OBJECTS:NoMocShared>)
set_property(TARGET FromObjects FromSharedObjects PROPERTY AUTOMOC OFF)

foreach(tgt IN ITEMS NoMocStatic NoMocObject NoMocShared NoMocExe)
  target_link_libraries(${tgt} PRIVATE Qt${with_qt_version}::Core)
  # The placeholder symbol is only compiled for Apple.
  if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${tgt} PRIVATE
      -Wall -Wextra -Werror
      $<IF:$<CXX_COMPILER_ID:GNU>,-Wmissing-declarations,-Wmissing-prototypes>
    )
  endif()
endforeach()
