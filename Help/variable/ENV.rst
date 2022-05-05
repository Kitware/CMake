ENV
---

Operator to read environment variables.

Use the syntax ``$ENV{VAR}`` to read environment variable ``VAR``.

To test whether an environment variable is defined, use the signature
``if(DEFINED ENV{<name>})`` of the :command:`if` command.

For general information on environment variables, see the
:ref:`Environment Variables <CMake Language Environment Variables>`
section in the :manual:`cmake-language(7)` manual.
