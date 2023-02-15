ninja-custom-command-depends
----------------------------

* The :command:`add_custom_command` command gained a new
  ``DEPENDS_EXPLICIT_ONLY`` option to tell the :ref:`Ninja Generators`
  not to add any dependencies implied by the target to which it is
  attached.
