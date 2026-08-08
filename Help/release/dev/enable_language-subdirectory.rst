enable_language-subdirectory
----------------------------

* The :command:`enable_language` command, and the :command:`project` command
  that calls it, may now be used in a subdirectory to enable a language for
  targets in ancestor directory scopes. The language configuration is
  propagated up to enclosing scopes. This also lifts the restriction for
  calling inside of :command:`block` and :command:`function` commands. See
  policy :policy:`CMP0220`.
