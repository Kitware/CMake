cmcmd-end-of-options-delimiter
------------------------------

* The :manual:`cmake(1)` ``-E`` commands ``cat`` and ``env`` learned to respect
  a double dash (``--``) argument that acts as a delimiter indicating the end of
  options. Any following arguments are treated as operands/positional arguments,
  even if they begin with a dash ``-`` character.
