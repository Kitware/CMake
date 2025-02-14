regex-fixes
-----------

* Regular expressions match the ``^`` anchor at most once in repeated
  searches, at the start of the input.  See policy :policy:`CMP0186`.

* References to unmatched groups are allowed, they are replaced with empty
  strings.
