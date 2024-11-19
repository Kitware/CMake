CLICOLOR
--------

.. versionadded:: 3.21

.. include:: ENV_VAR.txt

Set to ``0`` to tell command-line tools not to print color
messages even if connected to a terminal.
This is a `common convention`_ among command-line tools in general.

See also the :envvar:`CLICOLOR_FORCE` environment variable.
:envvar:`CLICOLOR_FORCE`, if activated, takes precedence over
:envvar:`!CLICOLOR`.

See the :variable:`CMAKE_COLOR_DIAGNOSTICS` variable to control
color in a generated build system.

.. _`common convention`: https://web.archive.org/web/20230417221418/https://bixense.com/clicolors/
