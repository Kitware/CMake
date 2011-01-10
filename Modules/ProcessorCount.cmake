# - ProcessorCount(var)
# Determine the number of processors/cores and save value in ${var}
#
# Sets the variable named ${var} to the number of physical cores available on
# the machine if the information can be determined. Otherwise it is set to 0.
# Currently this functionality is only implemented for Windows, Mac OS X and
# Unix systems providing getconf or the /proc/cpuinfo interface (e.g. Linux).

# A more reliable way might be to compile a small C program that uses the CPUID
# instruction, but that again requires compiler support or compiling assembler
# code.

#=============================================================================
# Copyright 2010 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

function(ProcessorCount var)
  # Unknown:
  set(count 0)

  if(WIN32)
    # Windows:
    set(count "$ENV{NUMBER_OF_PROCESSORS}")
    message("ProcessorCount: using environment variable")
  elseif(APPLE)
    # Mac:
    find_program(ProcessorCount_cmd_sysctl sysctl
      PATHS /usr/sbin)
    if(ProcessorCount_cmd_sysctl)
      execute_process(COMMAND ${ProcessorCount_cmd_sysctl} -n hw.ncpu
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE count)
      message("ProcessorCount: using sysctl '${ProcessorCount_cmd_sysctl}'")
    endif()
  else()
    # Linux (systems with getconf):
    find_program(ProcessorCount_cmd_getconf getconf)
    if(ProcessorCount_cmd_getconf)
      execute_process(COMMAND ${ProcessorCount_cmd_getconf} _NPROCESSORS_ONLN
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE count)
      message("ProcessorCount: using getconf '${ProcessorCount_cmd_getconf}'")
    endif()

    if(NOT count)
      # QNX (systems with pidin):
      find_program(ProcessorCount_cmd_pidin pidin)
      if(ProcessorCount_cmd_pidin)
        execute_process(COMMAND ${ProcessorCount_cmd_pidin} info
          OUTPUT_STRIP_TRAILING_WHITESPACE
          OUTPUT_VARIABLE pidin_output)
        string(REGEX MATCHALL "Processor[0-9]+: " procs "${pidin_output}")
        list(LENGTH procs count)
        message("ProcessorCount: using pidin '${ProcessorCount_cmd_pidin}'")
      endif()
    endif()
  endif()

  # Execute this code when there is no 'sysctl' or 'getconf' or 'pidin' or
  # when previously executed methods return empty output:
  #
  if(NOT count)
    # Systems with /proc/cpuinfo:
    set(cpuinfo_file /proc/cpuinfo)
    if(EXISTS "${cpuinfo_file}")
      file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
      list(LENGTH procs count)
      message("ProcessorCount: using cpuinfo '${cpuinfo_file}'")
    endif()
  endif()

  # Ensure an integer return (avoid inadvertently returning an empty string
  # or an error string)... If it's not a decimal integer, return 0:
  #
  if(NOT count MATCHES "^[0-9]+$")
    set(count 0)
  endif()

  set(${var} ${count} PARENT_SCOPE)
endfunction()
