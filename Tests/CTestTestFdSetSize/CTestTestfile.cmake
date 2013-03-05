# Test that more than 64 file handles are supported.
# If the default limit for Cygwin isn't increased tests may fail
# randomly with "BAD_COMMAND" or "TIMEOUT".

foreach (i 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20)
  add_test(Test${i} "sleep" "1")
endforeach ()
