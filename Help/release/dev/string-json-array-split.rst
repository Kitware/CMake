string-json-array-split
-----------------------

* The :command:`string(JSON)` command gained an ``ARRAY_SPLIT`` mode that
  splits a JSON array into a list of its elements with a single parse,
  enabling linear-time array iteration instead of re-parsing the whole
  JSON string for each element.
