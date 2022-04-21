werror-property
---------------

* Added the Target Property :prop_tgt:`COMPILE_WARNING_AS_ERROR` and the
  Variable :variable:`CMAKE_COMPILE_WARNING_AS_ERROR` which initializes the
  Target Property. If :prop_tgt:`COMPILE_WARNING_AS_ERROR` is true, it expands
  to a different flag depending on the compiler such that any warnings at
  compile will be treated as errors.
