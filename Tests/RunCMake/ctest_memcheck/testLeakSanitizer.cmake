# this file simulates a program that has been built with LeakSanitizer
# options

message("LSAN_OPTIONS = [$ENV{LSAN_OPTIONS}]")
string(REGEX REPLACE ".*log_path='([^']*)'.*" "\\1" LOG_FILE "$ENV{LSAN_OPTIONS}")
message("LOG_FILE=[${LOG_FILE}]")

# if we are not asked to simulate LeakSanitizer don't do it
if(NOT "$ENV{LSAN_OPTIONS}]" MATCHES "simulate_sanitizer.1")
  return()
endif()

# clear the log files
file(REMOVE "${LOG_FILE}.2343")
file(REMOVE "${LOG_FILE}.2344")

# create an error of each type of LeakSanitizer

file(APPEND "${LOG_FILE}.2343"
"=================================================================
==25308==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 4360 byte(s) in 1 object(s) allocated from:
    #0 0x46c669 in operator new[](unsigned long) (/home/kitware/msan/a.out+0x46c669)
    #1 0x4823b4 in main /home/kitware/msan/memcheck.cxx:12
    #2 0x7fa72bee476c in __libc_start_main /build/eglibc-2.15/csu/libc-start.c:226

SUMMARY: LeakSanitizer: 4436 byte(s) leaked in 2 allocation(s).
")
file(APPEND "${LOG_FILE}.2342"
"=================================================================
==25308==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 76 byte(s) in 1 object(s) allocated from:
    #0 0x46c669 in operator new[](unsigned long) (/home/kitware/msan/a.out+0x46c669)
    #1 0x4821b8 in foo() /home/kitware/msan/memcheck.cxx:4
    #2 0x4823f2 in main /home/kitware/msan/memcheck.cxx:14
    #3 0x7fa72bee476c in __libc_start_main /build/eglibc-2.15/csu/libc-start.c:226

Indirect leak of 76 byte(s) in 1 object(s) allocated from:
    #0 0x46c669 in operator new[](unsigned long) (/home/kitware/msan/a.out+0x46c669)
    #1 0x4821b8 in foo() /home/kitware/msan/memcheck.cxx:4
    #2 0x4823f2 in main /home/kitware/msan/memcheck.cxx:14
    #3 0x7fa72bee476c in __libc_start_main /build/eglibc-2.15/csu/libc-start.c:226

SUMMARY: LeakSanitizer: 4436 byte(s) leaked in 2 allocation(s).
")
