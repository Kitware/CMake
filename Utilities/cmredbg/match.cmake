cmake_minimum_required(VERSION 3.10)

if (NOT EXISTS "re.txt")
  message(FATAL_ERROR
    "Place your regular expression in `re.txt`.")
endif ()
if (NOT EXISTS "content.txt")
  message(FATAL_ERROR
    "Place your content in `content.txt`.")
endif ()

file(READ "re.txt" needle)
string(REGEX REPLACE "\n+$" "" needle "${needle}")
file(READ "content.txt" haystack)
string(REGEX REPLACE "\n+$" "" haystack "${haystack}")

if (haystack MATCHES "${needle}")
  message("Matches!")
else ()
  message("NO match!")
endif ()
