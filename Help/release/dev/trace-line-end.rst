trace-line-end
--------------

* Add the field ``line_end`` to the json-v1 trace format. This
  field tells you the line in file ``file`` at which the function
  call ends. Tools can use this new field, together with ``line``
  and ``file``, to map traces to lines of CMake source code.
