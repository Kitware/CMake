trace-global-frame
------------------

* Add the field ``global_frame`` to the json-v1 trace format. This
  frame tracks the depth of the call stack globally across all
  ``CMakeLists.txt`` files involved in the trace, and will let tools
  reconstruct stack traces that span from the top-level ``CMakeLists.txt``
  file of the project.
