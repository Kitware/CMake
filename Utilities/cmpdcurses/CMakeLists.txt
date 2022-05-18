project(PDCurses C)

if(NOT WIN32)
  message(FATAL_ERROR "PDCurses not (yet) supported on non-Windows platforms")
endif()

# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

add_library(cmpdcurses STATIC
  curses.h
  curspriv.h
  panel.h

  common/acs437.h
  common/acsuni.h

  pdcurses/addch.c
  pdcurses/addchstr.c
  pdcurses/addstr.c
  pdcurses/attr.c
  pdcurses/beep.c
  pdcurses/bkgd.c
  pdcurses/border.c
  pdcurses/clear.c
  pdcurses/color.c
  pdcurses/debug.c
  pdcurses/delch.c
  pdcurses/deleteln.c
  pdcurses/getch.c
  pdcurses/getstr.c
  pdcurses/getyx.c
  pdcurses/inch.c
  pdcurses/inchstr.c
  pdcurses/initscr.c
  pdcurses/inopts.c
  pdcurses/insch.c
  pdcurses/insstr.c
  pdcurses/instr.c
  pdcurses/kernel.c
  pdcurses/keyname.c
  pdcurses/mouse.c
  pdcurses/move.c
  pdcurses/outopts.c
  pdcurses/overlay.c
  pdcurses/pad.c
  pdcurses/panel.c
  pdcurses/printw.c
  pdcurses/refresh.c
  pdcurses/scanw.c
  pdcurses/scr_dump.c
  pdcurses/scroll.c
  pdcurses/slk.c
  pdcurses/termattr.c
  pdcurses/touch.c
  pdcurses/util.c
  pdcurses/window.c

  wincon/pdcclip.c
  wincon/pdcdisp.c
  wincon/pdcgetsc.c
  wincon/pdckbd.c
  wincon/pdcscrn.c
  wincon/pdcsetsc.c
  wincon/pdcutil.c
  wincon/pdcwin.h
  )
target_include_directories(cmpdcurses PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
