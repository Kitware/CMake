cmake_parse_arguments
---------------------

Parse function or macro arguments.

.. code-block:: cmake

  cmake_parse_arguments(<prefix> <options> <one_value_keywords>
                        <multi_value_keywords> <args>...)

  cmake_parse_arguments(PARSE_ARGV <N> <prefix> <options>
                        <one_value_keywords> <multi_value_keywords>)

.. versionadded:: 3.5
  This command is implemented natively.  Previously, it has been defined in the
  module :module:`CMakeParseArguments`.

This command is for use in macros or functions.
It processes the arguments given to that macro or function,
and defines a set of variables which hold the values of the
respective options.

The first signature reads arguments passed in the ``<args>...``.
This may be used in either a :command:`macro` or a :command:`function`.

.. versionadded:: 3.7
  The ``PARSE_ARGV`` signature is only for use in a :command:`function`
  body.  In this case, the arguments that are parsed come from the
  ``ARGV#`` variables of the calling function.  The parsing starts with
  the ``<N>``-th argument, where ``<N>`` is an unsigned integer.
  This allows for the values to have special characters like ``;`` in them.

The ``<options>`` argument contains all options for the respective function
or macro. These are keywords that have no value following them, like the
``OPTIONAL`` keyword of the :command:`install` command.

The ``<one_value_keywords>`` argument contains all keywords for this function
or macro which are followed by one value, like the ``DESTINATION`` keyword of
the :command:`install` command.

The ``<multi_value_keywords>`` argument contains all keywords for this
function or macro which can be followed by more than one value, like the
``TARGETS`` or ``FILES`` keywords of the :command:`install` command.

.. versionchanged:: 3.5
  All keywords must be unique.  Each keyword can only be specified
  once in any of the ``<options>``, ``<one_value_keywords>``, or
  ``<multi_value_keywords>``. A warning will be emitted if uniqueness is
  violated.

When done, ``cmake_parse_arguments`` will consider for each of the
keywords listed in ``<options>``, ``<one_value_keywords>``, and
``<multi_value_keywords>``, a variable composed of the given ``<prefix>``
followed by ``"_"`` and the name of the respective keyword.  For
``<one_value_keywords>`` and ``<multi_value_keywords>``, these variables
will then hold the respective value(s) from the argument list, or be undefined
if the associated keyword was not given (policy :policy:`CMP0174` can also
affect the behavior for ``<one_value_keywords>``).  For the ``<options>``
keywords, these variables will always be defined, and they will be set to
``TRUE`` if the keyword is present, or ``FALSE`` if it is not.

All remaining arguments are collected in a variable
``<prefix>_UNPARSED_ARGUMENTS`` that will be undefined if all arguments
were recognized. This can be checked afterwards to see
whether your macro or function was called with unrecognized parameters.

.. versionadded:: 3.15
   ``<one_value_keywords>`` and ``<multi_value_keywords>`` that were given no
   values at all are collected in a variable
   ``<prefix>_KEYWORDS_MISSING_VALUES`` that will be undefined if all keywords
   received values. This can be checked to see if there were keywords without
   any values given.

.. versionchanged:: 3.31
   If a ``<one_value_keyword>`` is followed by an empty string as its value,
   policy :policy:`CMP0174` controls whether a corresponding
   ``<prefix>_<keyword>`` variable is defined or not.


Consider the following example macro, ``my_install()``, which takes similar
arguments to the real :command:`install` command:

.. code-block:: cmake

   macro(my_install)
       set(options OPTIONAL FAST)
       set(oneValueArgs DESTINATION RENAME)
       set(multiValueArgs TARGETS CONFIGURATIONS)
       cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}"
                             "${multiValueArgs}" ${ARGN} )

       # ...

Assume ``my_install()`` has been called like this:

.. code-block:: cmake

   my_install(TARGETS foo bar DESTINATION bin OPTIONAL blub CONFIGURATIONS)

After the ``cmake_parse_arguments`` call, the macro will have set or undefined
the following variables::

   MY_INSTALL_OPTIONAL = TRUE
   MY_INSTALL_FAST = FALSE # was not used in call to my_install
   MY_INSTALL_DESTINATION = "bin"
   MY_INSTALL_RENAME <UNDEFINED> # was not used
   MY_INSTALL_TARGETS = "foo;bar"
   MY_INSTALL_CONFIGURATIONS <UNDEFINED> # was not used
   MY_INSTALL_UNPARSED_ARGUMENTS = "blub" # nothing expected after "OPTIONAL"
   MY_INSTALL_KEYWORDS_MISSING_VALUES = "CONFIGURATIONS"
            # No value for "CONFIGURATIONS" given

You can then continue and process these variables.

Keywords terminate lists of values. If a keyword is given directly after a
``<one_value_keyword>``, that preceding ``<one_value_keyword>`` receives no
value and the keyword is added to the ``<prefix>_KEYWORDS_MISSING_VALUES``
variable. For the above example, the call
``my_install(TARGETS foo DESTINATION OPTIONAL)`` would result in
``MY_INSTALL_OPTIONAL`` being set to ``TRUE`` and ``MY_INSTALL_DESTINATION``
being unset.  The ``MY_INSTALL_KEYWORDS_MISSING_VALUES`` variable would hold
the value ``DESTINATION``.

See Also
^^^^^^^^

* :command:`function`
* :command:`macro`
