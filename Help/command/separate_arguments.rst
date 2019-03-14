separate_arguments
------------------

Parse command-line arguments into a semicolon-separated list.

.. code-block:: cmake

  separate_arguments(<variable> <mode> <args>)

Parses a space-separated string ``<args>`` into a list of items,
and stores this list in semicolon-separated standard form in ``<variable>``.

This function is intended for parsing command-line arguments.
The entire command line must be passed as one string in the
argument ``<args>``.

The exact parsing rules depend on the operating system.
They are specified by the ``<mode>`` argument which must
be one of the following keywords:

``UNIX_COMMAND``
  Arguments are separated by by unquoted whitespace.
  Both single-quote and double-quote pairs are respected.
  A backslash escapes the next literal character (``\"`` is ``"``);
  there are no special escapes (``\n`` is just ``n``).

``WINDOWS_COMMAND``
  A Windows command-line is parsed using the same
  syntax the runtime library uses to construct argv at startup.  It
  separates arguments by whitespace that is not double-quoted.
  Backslashes are literal unless they precede double-quotes.  See the
  MSDN article `Parsing C Command-Line Arguments`_ for details.

``NATIVE_COMMAND``
  Proceeds as in ``WINDOWS_COMMAND`` mode if the host system is Windows.
  Otherwise proceeds as in ``UNIX_COMMAND`` mode.

.. _`Parsing C Command-Line Arguments`: https://msdn.microsoft.com/library/a1y7w461.aspx

.. code-block:: cmake

  separate_arguments(<var>)

Convert the value of ``<var>`` to a semi-colon separated list.  All
spaces are replaced with ';'.  This helps with generating command
lines.
