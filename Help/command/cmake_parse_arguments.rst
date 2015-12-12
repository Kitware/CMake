cmake_parse_arguments
---------------------

.. code-block:: cmake

  cmake_parse_arguments(<prefix> <options> <one_value_keywords>
                        <multi_value_keywords> args...)

``cmake_parse_arguments()`` is intended to be used in macros or functions
for parsing the arguments given to that macro or function.  It
processes the arguments and defines a set of variables which hold the
values of the respective options.

The ``<options>`` argument contains all options for the respective macro,
i.e.  keywords which can be used when calling the macro without any value
following, like e.g.  the ``OPTIONAL`` keyword of the :command:`install`
command.

The ``<one_value_keywords>`` argument contains all keywords for this macro
which are followed by one value, like e.g. ``DESTINATION`` keyword of the
:command:`install` command.

The ``<multi_value_keywords>`` argument contains all keywords for this
macro which can be followed by more than one value, like e.g. the
``TARGETS`` or ``FILES`` keywords of the :command:`install` command.

When done, ``cmake_parse_arguments()`` will have defined for each of the
keywords listed in ``<options>``, ``<one_value_keywords>`` and
``<multi_value_keywords>`` a variable composed of the given ``<prefix>``
followed by ``"_"`` and the name of the respective keyword.  These
variables will then hold the respective value from the argument list.
For the ``<options>`` keywords this will be ``TRUE`` or ``FALSE``.

All remaining arguments are collected in a variable
``<prefix>_UNPARSED_ARGUMENTS``, this can be checked afterwards to see
whether your macro was called with unrecognized parameters.

As an example here a ``my_install()`` macro, which takes similar arguments
as the real :command:`install` command:

.. code-block:: cmake

   function(MY_INSTALL)
       set(options OPTIONAL FAST)
       set(oneValueArgs DESTINATION RENAME)
       set(multiValueArgs TARGETS CONFIGURATIONS)
       cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}"
                             "${multiValueArgs}" ${ARGN} )

       # ...

Assume ``my_install()`` has been called like this:

.. code-block:: cmake

   my_install(TARGETS foo bar DESTINATION bin OPTIONAL blub)


After the ``cmake_parse_arguments()`` call the macro will have set the
following variables:

::

   MY_INSTALL_OPTIONAL = TRUE
   MY_INSTALL_FAST = FALSE (this option was not used when calling my_install()
   MY_INSTALL_DESTINATION = "bin"
   MY_INSTALL_RENAME = "" (was not used)
   MY_INSTALL_TARGETS = "foo;bar"
   MY_INSTALL_CONFIGURATIONS = "" (was not used)
   MY_INSTALL_UNPARSED_ARGUMENTS = "blub" (no value expected after "OPTIONAL"


You can then continue and process these variables.

Keywords terminate lists of values, e.g.  if directly after a
one_value_keyword another recognized keyword follows, this is
interpreted as the beginning of the new option.  E.g.
``my_install(TARGETS foo DESTINATION OPTIONAL)`` would result in
``MY_INSTALL_DESTINATION`` set to ``"OPTIONAL"``, but as ``OPTIONAL``
is a keyword itself ``MY_INSTALL_DESTINATION`` will be empty and
``MY_INSTALL_OPTIONAL`` will therefore be set to ``TRUE``.

