max-recursion-depth
-------------------

* CMake now imposes a maximum recursion limit to prevent a stack overflow on
  scripts that recurse infinitely. The limit can be adjusted at runtime with
  :variable:`CMAKE_MAXIMUM_RECURSION_DEPTH`.
