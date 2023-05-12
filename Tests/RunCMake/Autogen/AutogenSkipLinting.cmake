enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "$<1:${PSEUDO_IWYU}>" -some -args)
set(CMAKE_CXX_CLANG_TIDY "$<1:${PSEUDO_TIDY}>" -bad)
set(CMAKE_CXX_CPPLINT "$<1:${PSEUDO_CPPLINT}>" --error)
set(CMAKE_CXX_CPPCHECK "$<1:${PSEUDO_CPPCHECK}>" -error)

add_executable(SkipLinting SkipLinting.cxx SkipLinting.h)
set_source_files_properties(SkipLinting.cxx PROPERTIES SKIP_LINTING TRUE)

target_link_libraries(SkipLinting Qt${with_qt_version}::Core
                                   Qt${with_qt_version}::Widgets
                                   Qt${with_qt_version}::Gui)
