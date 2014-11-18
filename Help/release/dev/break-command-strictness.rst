break-command-strictness
------------------------

* The :command:`break` command now rejects calls outside of a loop
  context or that pass arguments to the command.
  See policy :policy:`CMP0055`.
