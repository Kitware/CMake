custom-command-byproducts
-------------------------

* The :command:`add_custom_command` and :command:`add_custom_target`
  commands learned a new ``BYPRODUCTS`` option to specify files
  produced as side effects of the custom commands.  These are not
  outputs because they do not always have to be newer than inputs.
