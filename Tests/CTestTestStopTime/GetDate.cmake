cmake_minimum_required(VERSION 2.2)

macro(GET_DATE)
  #
  # All macro arguments are optional.
  #   If there's an ARGV0, use it as GD_PREFIX. Default = 'GD_'
  #   If there's an ARGV1, use it as ${GD_PREFIX}VERBOSE. Default = '0'
  #
  # If the date can be retrieved and parsed successfully, this macro
  # will set the following CMake variables:
  #
  #   GD_PREFIX
  #   ${GD_PREFIX}PREFIX (if '${GD_PREFIX}' is not 'GD_'...!)
  #   ${GD_PREFIX}VERBOSE
  #
  #   ${GD_PREFIX}CMD
  #   ${GD_PREFIX}ARGS
  #   ${GD_PREFIX}OV
  #   ${GD_PREFIX}RV
  #
  #   ${GD_PREFIX}REGEX
  #   ${GD_PREFIX}YEAR
  #   ${GD_PREFIX}MONTH
  #   ${GD_PREFIX}DAY
  #   ${GD_PREFIX}HOUR
  #   ${GD_PREFIX}MINUTE
  #   ${GD_PREFIX}SECOND
  #   ${GD_PREFIX}FRACTIONAL_SECOND
  #   ${GD_PREFIX}DAY_OF_WEEK
  #
  # Caller can then use these variables to construct names based on
  # date and time stamps...
  #

  # If there's an ARGV0, use it as GD_PREFIX:
  #
  set(GD_PREFIX "GD_")
  if(NOT "${ARGV0}" STREQUAL "")
    set(GD_PREFIX "${ARGV0}")
  endif(NOT "${ARGV0}" STREQUAL "")
  if(NOT "${GD_PREFIX}" STREQUAL "GD_")
    set(${GD_PREFIX}PREFIX "${GD_PREFIX}")
  endif(NOT "${GD_PREFIX}" STREQUAL "GD_")

  # If there's an ARGV1, use it as ${GD_PREFIX}VERBOSE:
  #
  set(${GD_PREFIX}VERBOSE "0")
  if(NOT "${ARGV1}" STREQUAL "")
    set(${GD_PREFIX}VERBOSE "${ARGV1}")
  endif(NOT "${ARGV1}" STREQUAL "")

  # Retrieve the current date and time in the format:
  #
  # Thu 01/12/2006  8:55:12.01
  # dow mm/dd/YYYY HH:MM:SS.ssssss
  #
  # Use "echo %DATE% %TIME%" on Windows.
  # Otherwise, try "date" as implemented on most Unix flavors.
  #
  if(WIN32)
    #
    # Use "cmd" shell with %DATE% and %TIME% support...
    # May need adjustment in different locales or for custom date/time formats
    # set in the Windows Control Panel.
    #
    set(${GD_PREFIX}CMD "cmd")
    set(${GD_PREFIX}ARGS "/c echo %DATE% %TIME%")
  else(WIN32)
    #
    # Match the format returned by default in US English Windows:
    #
    set(${GD_PREFIX}CMD "date")
    set(${GD_PREFIX}ARGS "\"+%a %m/%d/%Y %H:%M:%S.00\"")
  endif(WIN32)

  exec_program("${${GD_PREFIX}CMD}" "." ARGS "${${GD_PREFIX}ARGS}"
    OUTPUT_VARIABLE ${GD_PREFIX}OV RETURN_VALUE ${GD_PREFIX}RV
    )

  if(${GD_PREFIX}VERBOSE)
    message(STATUS "")
    message(STATUS "<GET_DATE>")
    message(STATUS "")
    message(STATUS "GD_PREFIX='${GD_PREFIX}'")
    if(NOT "${GD_PREFIX}" STREQUAL "GD_")
      message(STATUS "${GD_PREFIX}PREFIX='${${GD_PREFIX}PREFIX}'")
    endif(NOT "${GD_PREFIX}" STREQUAL "GD_")
    message(STATUS "${GD_PREFIX}VERBOSE='${${GD_PREFIX}VERBOSE}'")
    message(STATUS "")
    message(STATUS "${GD_PREFIX}CMD='${${GD_PREFIX}CMD}'")
    message(STATUS "${GD_PREFIX}ARGS='${${GD_PREFIX}ARGS}'")
    message(STATUS "${GD_PREFIX}OV='${${GD_PREFIX}OV}'")
    message(STATUS "${GD_PREFIX}RV='${${GD_PREFIX}RV}'")
    message(STATUS "")
  endif(${GD_PREFIX}VERBOSE)

  if("${${GD_PREFIX}RV}" STREQUAL "0")
    #
    # Extract eight individual components by matching a regex with paren groupings.
    # Use the replace functionality and \\1 thru \\8 to extract components.
    #
    set(${GD_PREFIX}REGEX "([^ ]+) +([^/]+)/([^/]+)/([^ ]+) +([^:]+):([^:]+):([^\\.]+)\\.(.*)")

    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\1" ${GD_PREFIX}DAY_OF_WEEK "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\2" ${GD_PREFIX}MONTH "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\3" ${GD_PREFIX}DAY "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\4" ${GD_PREFIX}YEAR "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\5" ${GD_PREFIX}HOUR "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\6" ${GD_PREFIX}MINUTE "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\7" ${GD_PREFIX}SECOND "${${GD_PREFIX}OV}")
    string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\8" ${GD_PREFIX}FRACTIONAL_SECOND "${${GD_PREFIX}OV}")

    #
    # Verify that extracted components don't have anything obviously
    # wrong with them... Emit warnings if something looks suspicious...
    #

    # Expecting a four digit year:
    #
    if(NOT "${${GD_PREFIX}YEAR}" MATCHES "^[0-9][0-9][0-9][0-9]$")
      message(STATUS "WARNING: Extracted ${GD_PREFIX}YEAR='${${GD_PREFIX}YEAR}' is not a four digit number...")
    endif(NOT "${${GD_PREFIX}YEAR}" MATCHES "^[0-9][0-9][0-9][0-9]$")

    # Expecting month to be <= 12:
    #
    if(${${GD_PREFIX}MONTH} GREATER 12)
      message(STATUS "WARNING: Extracted ${GD_PREFIX}MONTH='${${GD_PREFIX}MONTH}' is greater than 12!")
    endif(${${GD_PREFIX}MONTH} GREATER 12)

    # Expecting day to be <= 31:
    #
    if(${${GD_PREFIX}DAY} GREATER 31)
      message(STATUS "WARNING: Extracted ${GD_PREFIX}DAY='${${GD_PREFIX}DAY}' is greater than 31!")
    endif(${${GD_PREFIX}DAY} GREATER 31)

    # Expecting hour to be <= 23:
    #
    if(${${GD_PREFIX}HOUR} GREATER 23)
      message(STATUS "WARNING: Extracted ${GD_PREFIX}HOUR='${${GD_PREFIX}HOUR}' is greater than 23!")
    endif(${${GD_PREFIX}HOUR} GREATER 23)

    # Expecting minute to be <= 59:
    #
    if(${${GD_PREFIX}MINUTE} GREATER 59)
      message(STATUS "WARNING: Extracted ${GD_PREFIX}MINUTE='${${GD_PREFIX}MINUTE}' is greater than 59!")
    endif(${${GD_PREFIX}MINUTE} GREATER 59)

    # Expecting second to be <= 59:
    #
    if(${${GD_PREFIX}SECOND} GREATER 59)
      message(STATUS "WARNING: Extracted ${GD_PREFIX}SECOND='${${GD_PREFIX}SECOND}' is greater than 59!")
    endif(${${GD_PREFIX}SECOND} GREATER 59)

    # If individual components are single digit,
    # prepend a leading zero:
    #
    if("${${GD_PREFIX}YEAR}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}YEAR "0${${GD_PREFIX}YEAR}")
    endif("${${GD_PREFIX}YEAR}" MATCHES "^[0-9]$")
    if("${${GD_PREFIX}MONTH}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}MONTH "0${${GD_PREFIX}MONTH}")
    endif("${${GD_PREFIX}MONTH}" MATCHES "^[0-9]$")
    if("${${GD_PREFIX}DAY}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}DAY "0${${GD_PREFIX}DAY}")
    endif("${${GD_PREFIX}DAY}" MATCHES "^[0-9]$")
    if("${${GD_PREFIX}HOUR}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}HOUR "0${${GD_PREFIX}HOUR}")
    endif("${${GD_PREFIX}HOUR}" MATCHES "^[0-9]$")
    if("${${GD_PREFIX}MINUTE}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}MINUTE "0${${GD_PREFIX}MINUTE}")
    endif("${${GD_PREFIX}MINUTE}" MATCHES "^[0-9]$")
    if("${${GD_PREFIX}SECOND}" MATCHES "^[0-9]$")
      set(${GD_PREFIX}SECOND "0${${GD_PREFIX}SECOND}")
    endif("${${GD_PREFIX}SECOND}" MATCHES "^[0-9]$")

    if(${GD_PREFIX}VERBOSE)
      message(STATUS "${GD_PREFIX}REGEX='${${GD_PREFIX}REGEX}'")
      message(STATUS "${GD_PREFIX}YEAR='${${GD_PREFIX}YEAR}'")
      message(STATUS "${GD_PREFIX}MONTH='${${GD_PREFIX}MONTH}'")
      message(STATUS "${GD_PREFIX}DAY='${${GD_PREFIX}DAY}'")
      message(STATUS "${GD_PREFIX}HOUR='${${GD_PREFIX}HOUR}'")
      message(STATUS "${GD_PREFIX}MINUTE='${${GD_PREFIX}MINUTE}'")
      message(STATUS "${GD_PREFIX}SECOND='${${GD_PREFIX}SECOND}'")
      message(STATUS "${GD_PREFIX}FRACTIONAL_SECOND='${${GD_PREFIX}FRACTIONAL_SECOND}'")
      message(STATUS "${GD_PREFIX}DAY_OF_WEEK='${${GD_PREFIX}DAY_OF_WEEK}'")
      message(STATUS "")
      message(STATUS "Counters that change...")
      message(STATUS "")
      message(STATUS "...very very quickly : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}${${GD_PREFIX}HOUR}${${GD_PREFIX}MINUTE}${${GD_PREFIX}SECOND}${${GD_PREFIX}FRACTIONAL_SECOND}")
      message(STATUS "        every second : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}${${GD_PREFIX}HOUR}${${GD_PREFIX}MINUTE}${${GD_PREFIX}SECOND}")
      message(STATUS "               daily : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}")
      message(STATUS "             monthly : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}")
      message(STATUS "            annually : ${${GD_PREFIX}YEAR}")
      message(STATUS "")
    endif(${GD_PREFIX}VERBOSE)
  else("${${GD_PREFIX}RV}" STREQUAL "0")
    message(SEND_ERROR "ERROR: macro(GET_DATE) failed. ${GD_PREFIX}CMD='${${GD_PREFIX}CMD}' ${GD_PREFIX}ARGS='${${GD_PREFIX}ARGS}' ${GD_PREFIX}OV='${${GD_PREFIX}OV}' ${GD_PREFIX}RV='${${GD_PREFIX}RV}'")
  endif("${${GD_PREFIX}RV}" STREQUAL "0")

  if(${GD_PREFIX}VERBOSE)
    message(STATUS "</GET_DATE>")
    message(STATUS "")
  endif(${GD_PREFIX}VERBOSE)
endmacro(GET_DATE)

macro(ADD_SECONDS sec)
  set(new_min ${${GD_PREFIX}MINUTE})
  set(new_hr ${${GD_PREFIX}HOUR})
  math(EXPR new_sec "${sec} + ${${GD_PREFIX}SECOND}")
  while(${new_sec} GREATER 60 OR ${new_sec} EQUAL 60)
    math(EXPR new_sec "${new_sec} - 60")
    math(EXPR new_min "${${GD_PREFIX}MINUTE} + 1")
  endwhile()
  while(${new_min} GREATER 60 OR ${new_min} EQUAL 60)
    math(EXPR new_min "${new_min} - 60")
    math(EXPR new_hr "${${GD_PREFIX}HOUR} + 1")
  endwhile()
  math(EXPR new_hr "${new_hr} % 24")

  # Pad the H, M, S if needed
  string(LENGTH ${new_sec} sec_len)
  string(LENGTH ${new_min} min_len)
  string(LENGTH ${new_hr} hr_len)
  if(${sec_len} EQUAL 1)
    set(new_sec "0${new_sec}")
  endif()
  if(${min_len} EQUAL 1)
    set(new_min "0${new_min}")
  endif()
  if(${hr_len} EQUAL 1)
    set(new_hr "0${new_hr}")
  endif()
endmacro(ADD_SECONDS)
