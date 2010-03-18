CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

MACRO(GET_DATE)
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
  SET(GD_PREFIX "GD_")
  IF(NOT "${ARGV0}" STREQUAL "")
    SET(GD_PREFIX "${ARGV0}")
  ENDIF(NOT "${ARGV0}" STREQUAL "")
  IF(NOT "${GD_PREFIX}" STREQUAL "GD_")
    SET(${GD_PREFIX}PREFIX "${GD_PREFIX}")
  ENDIF(NOT "${GD_PREFIX}" STREQUAL "GD_")

  # If there's an ARGV1, use it as ${GD_PREFIX}VERBOSE:
  #
  SET(${GD_PREFIX}VERBOSE "0")
  IF(NOT "${ARGV1}" STREQUAL "")
    SET(${GD_PREFIX}VERBOSE "${ARGV1}")
  ENDIF(NOT "${ARGV1}" STREQUAL "")

  # Retrieve the current date and time in the format:
  #
  # Thu 01/12/2006  8:55:12.01
  # dow mm/dd/YYYY HH:MM:SS.ssssss
  #
  # Use "echo %DATE% %TIME%" on Windows.
  # Otherwise, try "date" as implemented on most Unix flavors.
  #
  IF(WIN32)
    #
    # Use "cmd" shell with %DATE% and %TIME% support...
    # May need adjustment in different locales or for custom date/time formats
    # set in the Windows Control Panel.
    #
    SET(${GD_PREFIX}CMD "cmd")
    SET(${GD_PREFIX}ARGS "/c echo %DATE% %TIME%")
  ELSE(WIN32)
    #
    # Match the format returned by default in US English Windows:
    #
    SET(${GD_PREFIX}CMD "date")
    SET(${GD_PREFIX}ARGS "\"+%a %m/%d/%Y %H:%M:%S.00\"")
  ENDIF(WIN32)

  EXEC_PROGRAM("${${GD_PREFIX}CMD}" "." ARGS "${${GD_PREFIX}ARGS}"
    OUTPUT_VARIABLE ${GD_PREFIX}OV RETURN_VALUE ${GD_PREFIX}RV
    )

  IF(${GD_PREFIX}VERBOSE)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "<GET_DATE>")
    MESSAGE(STATUS "")
    MESSAGE(STATUS "GD_PREFIX='${GD_PREFIX}'")
    IF(NOT "${GD_PREFIX}" STREQUAL "GD_")
      MESSAGE(STATUS "${GD_PREFIX}PREFIX='${${GD_PREFIX}PREFIX}'")
    ENDIF(NOT "${GD_PREFIX}" STREQUAL "GD_")
    MESSAGE(STATUS "${GD_PREFIX}VERBOSE='${${GD_PREFIX}VERBOSE}'")
    MESSAGE(STATUS "")
    MESSAGE(STATUS "${GD_PREFIX}CMD='${${GD_PREFIX}CMD}'")
    MESSAGE(STATUS "${GD_PREFIX}ARGS='${${GD_PREFIX}ARGS}'")
    MESSAGE(STATUS "${GD_PREFIX}OV='${${GD_PREFIX}OV}'")
    MESSAGE(STATUS "${GD_PREFIX}RV='${${GD_PREFIX}RV}'")
    MESSAGE(STATUS "")
  ENDIF(${GD_PREFIX}VERBOSE)

  IF("${${GD_PREFIX}RV}" STREQUAL "0")
    #
    # Extract eight individual components by matching a regex with paren groupings.
    # Use the replace functionality and \\1 thru \\8 to extract components.
    #
    SET(${GD_PREFIX}REGEX "([^ ]+) +([^/]+)/([^/]+)/([^ ]+) +([^:]+):([^:]+):([^\\.]+)\\.(.*)")

    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\1" ${GD_PREFIX}DAY_OF_WEEK "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\2" ${GD_PREFIX}MONTH "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\3" ${GD_PREFIX}DAY "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\4" ${GD_PREFIX}YEAR "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\5" ${GD_PREFIX}HOUR "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\6" ${GD_PREFIX}MINUTE "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\7" ${GD_PREFIX}SECOND "${${GD_PREFIX}OV}")
    STRING(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\8" ${GD_PREFIX}FRACTIONAL_SECOND "${${GD_PREFIX}OV}")

    #
    # Verify that extracted components don't have anything obviously
    # wrong with them... Emit warnings if something looks suspicious...
    #

    # Expecting a four digit year:
    #
    IF(NOT "${${GD_PREFIX}YEAR}" MATCHES "^[0-9][0-9][0-9][0-9]$")
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}YEAR='${${GD_PREFIX}YEAR}' is not a four digit number...")
    ENDIF(NOT "${${GD_PREFIX}YEAR}" MATCHES "^[0-9][0-9][0-9][0-9]$")

    # Expecting month to be <= 12:
    #
    IF(${${GD_PREFIX}MONTH} GREATER 12)
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}MONTH='${${GD_PREFIX}MONTH}' is greater than 12!")
    ENDIF(${${GD_PREFIX}MONTH} GREATER 12)

    # Expecting day to be <= 31:
    #
    IF(${${GD_PREFIX}DAY} GREATER 31)
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}DAY='${${GD_PREFIX}DAY}' is greater than 31!")
    ENDIF(${${GD_PREFIX}DAY} GREATER 31)

    # Expecting hour to be <= 23:
    #
    IF(${${GD_PREFIX}HOUR} GREATER 23)
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}HOUR='${${GD_PREFIX}HOUR}' is greater than 23!")
    ENDIF(${${GD_PREFIX}HOUR} GREATER 23)

    # Expecting minute to be <= 59:
    #
    IF(${${GD_PREFIX}MINUTE} GREATER 59)
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}MINUTE='${${GD_PREFIX}MINUTE}' is greater than 59!")
    ENDIF(${${GD_PREFIX}MINUTE} GREATER 59)

    # Expecting second to be <= 59:
    #
    IF(${${GD_PREFIX}SECOND} GREATER 59)
      MESSAGE(STATUS "WARNING: Extracted ${GD_PREFIX}SECOND='${${GD_PREFIX}SECOND}' is greater than 59!")
    ENDIF(${${GD_PREFIX}SECOND} GREATER 59)

    # If individual components are single digit,
    # prepend a leading zero:
    #
    IF("${${GD_PREFIX}YEAR}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}YEAR "0${${GD_PREFIX}YEAR}")
    ENDIF("${${GD_PREFIX}YEAR}" MATCHES "^[0-9]$")
    IF("${${GD_PREFIX}MONTH}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}MONTH "0${${GD_PREFIX}MONTH}")
    ENDIF("${${GD_PREFIX}MONTH}" MATCHES "^[0-9]$")
    IF("${${GD_PREFIX}DAY}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}DAY "0${${GD_PREFIX}DAY}")
    ENDIF("${${GD_PREFIX}DAY}" MATCHES "^[0-9]$")
    IF("${${GD_PREFIX}HOUR}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}HOUR "0${${GD_PREFIX}HOUR}")
    ENDIF("${${GD_PREFIX}HOUR}" MATCHES "^[0-9]$")
    IF("${${GD_PREFIX}MINUTE}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}MINUTE "0${${GD_PREFIX}MINUTE}")
    ENDIF("${${GD_PREFIX}MINUTE}" MATCHES "^[0-9]$")
    IF("${${GD_PREFIX}SECOND}" MATCHES "^[0-9]$")
      SET(${GD_PREFIX}SECOND "0${${GD_PREFIX}SECOND}")
    ENDIF("${${GD_PREFIX}SECOND}" MATCHES "^[0-9]$")

    IF(${GD_PREFIX}VERBOSE)
      MESSAGE(STATUS "${GD_PREFIX}REGEX='${${GD_PREFIX}REGEX}'")
      MESSAGE(STATUS "${GD_PREFIX}YEAR='${${GD_PREFIX}YEAR}'")
      MESSAGE(STATUS "${GD_PREFIX}MONTH='${${GD_PREFIX}MONTH}'")
      MESSAGE(STATUS "${GD_PREFIX}DAY='${${GD_PREFIX}DAY}'")
      MESSAGE(STATUS "${GD_PREFIX}HOUR='${${GD_PREFIX}HOUR}'")
      MESSAGE(STATUS "${GD_PREFIX}MINUTE='${${GD_PREFIX}MINUTE}'")
      MESSAGE(STATUS "${GD_PREFIX}SECOND='${${GD_PREFIX}SECOND}'")
      MESSAGE(STATUS "${GD_PREFIX}FRACTIONAL_SECOND='${${GD_PREFIX}FRACTIONAL_SECOND}'")
      MESSAGE(STATUS "${GD_PREFIX}DAY_OF_WEEK='${${GD_PREFIX}DAY_OF_WEEK}'")
      MESSAGE(STATUS "")
      MESSAGE(STATUS "Counters that change...")
      MESSAGE(STATUS "")
      MESSAGE(STATUS "...very very quickly : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}${${GD_PREFIX}HOUR}${${GD_PREFIX}MINUTE}${${GD_PREFIX}SECOND}${${GD_PREFIX}FRACTIONAL_SECOND}")
      MESSAGE(STATUS "        every second : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}${${GD_PREFIX}HOUR}${${GD_PREFIX}MINUTE}${${GD_PREFIX}SECOND}")
      MESSAGE(STATUS "               daily : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}")
      MESSAGE(STATUS "             monthly : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}")
      MESSAGE(STATUS "            annually : ${${GD_PREFIX}YEAR}")
      MESSAGE(STATUS "")
    ENDIF(${GD_PREFIX}VERBOSE)
  ELSE("${${GD_PREFIX}RV}" STREQUAL "0")
    MESSAGE(SEND_ERROR "ERROR: MACRO(GET_DATE) failed. ${GD_PREFIX}CMD='${${GD_PREFIX}CMD}' ${GD_PREFIX}ARGS='${${GD_PREFIX}ARGS}' ${GD_PREFIX}OV='${${GD_PREFIX}OV}' ${GD_PREFIX}RV='${${GD_PREFIX}RV}'")
  ENDIF("${${GD_PREFIX}RV}" STREQUAL "0")

  IF(${GD_PREFIX}VERBOSE)
    MESSAGE(STATUS "</GET_DATE>")
    MESSAGE(STATUS "")
  ENDIF(${GD_PREFIX}VERBOSE)
ENDMACRO(GET_DATE)

MACRO(ADD_SECONDS sec)
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
ENDMACRO(ADD_SECONDS)
