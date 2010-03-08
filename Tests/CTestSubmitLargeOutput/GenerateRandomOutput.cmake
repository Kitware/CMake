#
# This script generates random lines of output.
#
# By default, it generates 100M of output (a million lines of 100 bytes each),
# but you can override that by passing in -D line_count and/or -D line_size...
#

# Default values:
#
if(NOT DEFINED line_count)
  set(line_count 1000000)
endif()

if(NOT DEFINED line_size)
  set(line_size 100)
endif()

if(NOT DEFINED random_seed)
  set(random_seed 1987)
endif()

# Use RANDOM_SEED once before the loop:
#
string(RANDOM LENGTH ${line_size} RANDOM_SEED ${random_seed} s)

# Emit line_count lines of random output:
#
foreach(i RANGE 1 ${line_count})
  string(RANDOM LENGTH ${line_size} s)
  message(${s})
endforeach()
