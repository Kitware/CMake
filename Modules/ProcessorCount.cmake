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
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

function(ProcessorCount var)
  # Unknown:
  set(count 0)

  if(WIN32)
    # Windows:
    set(count "$ENV{NUMBER_OF_PROCESSORS}")
  elseif(APPLE)
    # Mac:
    find_program(ProcessorCount_cmd_sysctl sysctl
      PATHS /usr/sbin)
    if(ProcessorCount_cmd_sysctl)
      execute_process(COMMAND ${ProcessorCount_cmd_sysctl} -n hw.ncpu
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE count)
    endif()
  else()
    find_program(ProcessorCount_cmd_getconf getconf)
    if(ProcessorCount_cmd_getconf)
      # Linux and other systems with getconf:
      execute_process(COMMAND ${ProcessorCount_cmd_getconf} _NPROCESSORS_ONLN
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE count)
    else()
      # Linux and other systems with /proc/cpuinfo:
      set(cpuinfo_file /proc/cpuinfo)
      if(EXISTS "${cpuinfo_file}")
        file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
        list(LENGTH procs count)
      endif()
    endif()
  endif()

  set(${var} ${count} PARENT_SCOPE)
endfunction()
