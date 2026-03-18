CMAKE_INSTALL_DEFAULT_COMPONENT_NAME
------------------------------------

Default component used in :command:`install` commands.

If an :command:`install` command is used without the ``COMPONENT`` argument,
these files will be grouped into a default component.  The name of this
default install component will be taken from this variable.  It
defaults to ``Unspecified``.

There is a special component name:

``<PROJECT_NAME>``
  .. versionadded:: 4.4

  This literal placeholder will expand to the contents of the
  :variable:`PROJECT_NAME` variable when the :command:`install` command is
  evaluated. If no :command:`project` command has been called it defaults to
  ``Unspecified``.
