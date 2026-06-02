parse-arguments-argn
--------------------

* The :command:`cmake_parse_arguments` command gained a new ``PARSE_ARGN``
  signature that starts parsing after the last named argument of the calling
  function and works exactly like ``PARSE_ARGV`` with ``<N>`` being the number
  of parameters in the function definition.
