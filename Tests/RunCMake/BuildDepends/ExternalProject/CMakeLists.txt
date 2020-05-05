cmake_minimum_required(VERSION 3.12)
project(External NONE)

if (DEFINED cache_arg)
  message("configured with: ${cache_arg}")
else ()
  message("cache_arg is undefined")
endif ()

if (DEFINED second_cache_arg)
  message("configured again with: ${second_cache_arg}")
else ()
  message("second_cache_arg is undefined")
endif ()
