CLICOLOR_FORCE
--------------

.. versionadded:: 3.5

.. include:: ENV_VAR.txt

Set to a non-empty value, other than ``0``, to tell command-line
tools to print color messages even if not connected to a terminal.
This is a `common convention`_ among command-line tools in general.

See also the :envvar:`CLICOLOR` environment variable.
:envvar:`!CLICOLOR_FORCE`, if activated, takes precedence over
:envvar:`CLICOLOR`.

See the :variable:`CMAKE_COLOR_DIAGNOSTICS` variable to control
color in a generated build system.

.. _`common convention`: https://web.archive.org/web/20230417221418/https://bixense.com/clicolors/
