cmake_minimum_required(VERSION 3.13...3.24 FATAL_ERROR)
project(CheckCurses C)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
if(NOT CURSES_FOUND)
  return()
endif()
include_directories(${CURSES_INCLUDE_DIRS})
add_executable(CheckCurses CheckCurses.c)
target_link_libraries(CheckCurses ${CURSES_LIBRARIES})

foreach(h
    CURSES_HAVE_CURSES_H
    CURSES_HAVE_NCURSES_H
    CURSES_HAVE_NCURSES_NCURSES_H
    CURSES_HAVE_NCURSES_CURSES_H
    )
  if(${h})
    target_compile_definitions(CheckCurses PRIVATE ${h})
  endif()
endforeach()
