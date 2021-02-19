list-index-arg-parsing
----------------------

* The :command:`list` command's ``GET``, ``INSERT``, ``SUBLIST``, and
  ``REMOVE_AT`` subcommands now error with invalid (i.e., non-integer) values
  are given as any of their index arguments based on the setting of policy
  :policy:`CMP0121`.
