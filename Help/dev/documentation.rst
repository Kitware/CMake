CMake Documentation Guide
*************************

The following is a guide to the CMake documentation source for developers.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

Help
====

The ``Help`` directory contains CMake help manual source files.
They are written using the `reStructuredText`_ markup syntax and
processed by `Sphinx`_ to generate the CMake help manuals.

.. _`reStructuredText`: http://docutils.sourceforge.net/docs/ref/rst/introduction.html
.. _`Sphinx`: http://sphinx-doc.org

Markup Constructs
-----------------

In addition to using Sphinx to generate the CMake help manuals, we
also use a C++-implemented document processor to print documents for
the ``--help-*`` command-line help options.  It supports a subset of
reStructuredText markup.  When authoring or modifying documents,
please verify that the command-line help looks good in addition to the
Sphinx-generated html and man pages.

The command-line help processor supports the following constructs
defined by reStructuredText, Sphinx, and a CMake extension to Sphinx.

..
 Note: This list must be kept consistent with the cmRST implementation.

CMake Domain directives
 Directives defined in the `CMake Domain`_ for defining CMake
 documentation objects are printed in command-line help output as
 if the lines were normal paragraph text with interpretation.

CMake Domain interpreted text roles
 Interpreted text roles defined in the `CMake Domain`_ for
 cross-referencing CMake documentation objects are replaced by their
 link text in command-line help output.  Other roles are printed
 literally and not processed.

``code-block`` directive
 Add a literal code block without interpretation.  The command-line
 help processor prints the block content without the leading directive
 line and with common indentation replaced by one space.

``include`` directive
 Include another document source file.  The command-line help
 processor prints the included document inline with the referencing
 document.

literal block after ``::``
 A paragraph ending in ``::`` followed by a blank line treats
 the following indented block as literal text without interpretation.
 The command-line help processor prints the ``::`` literally and
 prints the block content with common indentation replaced by one
 space.

``note`` directive
 Call out a side note.  The command-line help processor prints the
 block content as if the lines were normal paragraph text with
 interpretation.

``parsed-literal`` directive
 Add a literal block with markup interpretation.  The command-line
 help processor prints the block content without the leading
 directive line and with common indentation replaced by one space.

``productionlist`` directive
 Render context-free grammar productions.  The command-line help
 processor prints the block content as if the lines were normal
 paragraph text with interpretation.

``replace`` directive
 Define a ``|substitution|`` replacement.
 The command-line help processor requires a substitution replacement
 to be defined before it is referenced.

``|substitution|`` reference
 Reference a substitution replacement previously defined by
 the ``replace`` directive.  The command-line help processor
 performs the substitution and replaces all newlines in the
 replacement text with spaces.

``toctree`` directive
 Include other document sources in the Table-of-Contents
 document tree.  The command-line help processor prints
 the referenced documents inline as part of the referencing
 document.

``versionadded``, ``versionchanged`` directives
 Specify that something was added or changed by a named CMake version.
 The command-line help processor prints the block content as if the lines
 were normal paragraph text with interpretation.

Inline markup constructs not listed above are printed literally in the
command-line help output.  We prefer to use inline markup constructs that
look correct in source form, so avoid use of \\-escapes in favor of inline
literals when possible.

Explicit markup blocks not matching directives listed above are removed from
command-line help output.  Do not use them, except for plain ``..`` comments
that are removed by Sphinx too.

Note that nested indentation of blocks is not recognized by the
command-line help processor.  Therefore:

* Explicit markup blocks are recognized only when not indented
  inside other blocks.

* Literal blocks after paragraphs ending in ``::`` but not
  at the top indentation level may consume all indented lines
  following them.

Try to avoid these cases in practice.

CMake Domain
------------

CMake adds a `Sphinx Domain`_ called ``cmake``, also called the
"CMake Domain".  It defines several "object" types for CMake
documentation:

``command``
 A CMake language command.

``cpack_gen``
 A CPack package generator.
 See the `cpack(1)`_ command-line tool's ``-G`` option.

``envvar``
 An environment variable.
 See the `cmake-env-variables(7)`_ manual
 and the `set()`_ command.

``generator``
 A CMake native build system generator.
 See the `cmake(1)`_ command-line tool's ``-G`` option.

``genex``
 A CMake generator expression.
 See the `cmake-generator-expressions(7)`_ manual.

``manual``
 A CMake manual page, like the `cmake(1)`_ manual.

``module``
 A CMake module.
 See the `cmake-modules(7)`_ manual
 and the `include()`_ command.

``policy``
 A CMake policy.
 See the `cmake-policies(7)`_ manual
 and the `cmake_policy()`_ command.

``prop_cache, prop_dir, prop_gbl, prop_sf, prop_inst, prop_test, prop_tgt``
 A CMake cache, directory, global, source file, installed file, test,
 or target property, respectively.  See the `cmake-properties(7)`_
 manual and the `set_property()`_ command.

``variable``
 A CMake language variable.
 See the `cmake-variables(7)`_ manual
 and the `set()`_ command.

Documentation objects in the CMake Domain come from two sources.
First, the CMake extension to Sphinx transforms every document named
with the form ``Help/<type>/<file-name>.rst`` to a domain object with
type ``<type>``.  The object name is extracted from the document title,
which is expected to be of the form::

 <object-name>
 -------------

and to appear at or near the top of the ``.rst`` file before any other
lines starting in a letter, digit, ``<``, or ``$``.  If no such title appears
literally in the ``.rst`` file, the object name is the ``<file-name>``.
If a title does appear, it is expected that ``<file-name>`` is equal
to ``<object-name>`` with any ``<`` and ``>`` characters removed,
or in the case of a ``$<genex-name>`` or ``$<genex-name:...>``, the
``genex-name``.

Second, the CMake Domain provides directives to define objects inside
other documents:

.. code-block:: rst

 .. command:: <command-name>

  This indented block documents <command-name>.

 .. envvar:: <envvar-name>

  This indented block documents <envvar-name>.

 .. genex:: <genex-name>

  This indented block documents <genex-name>.

 .. variable:: <variable-name>

  This indented block documents <variable-name>.

Object types for which no directive is available must be defined using
the first approach above.

.. _`Sphinx Domain`: http://sphinx-doc.org/domains.html
.. _`cmake(1)`: https://cmake.org/cmake/help/latest/manual/cmake.1.html
.. _`cmake-env-variables(7)`: https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html
.. _`cmake-generator-expressions(7)`: https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
.. _`cmake-modules(7)`: https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html
.. _`cmake-policies(7)`: https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html
.. _`cmake-properties(7)`: https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html
.. _`cmake-variables(7)`: https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html
.. _`cmake_policy()`: https://cmake.org/cmake/help/latest/command/cmake_policy.html
.. _`cpack(1)`: https://cmake.org/cmake/help/latest/manual/cpack.1.html
.. _`include()`: https://cmake.org/cmake/help/latest/command/include.html
.. _`set()`: https://cmake.org/cmake/help/latest/command/set.html
.. _`set_property()`: https://cmake.org/cmake/help/latest/command/set_property.html

Cross-References
----------------

Sphinx uses reStructuredText interpreted text roles to provide
cross-reference syntax.  The `CMake Domain`_ provides for each
domain object type a role of the same name to cross-reference it.
CMake Domain roles are inline markup of the forms::

 :type:`name`
 :type:`text <name>`

where ``type`` is the domain object type and ``name`` is the
domain object name.  In the first form the link text will be
``name`` (or ``name()`` if the type is ``command``) and in
the second form the link text will be the explicit ``text``.
For example, the code:

.. code-block:: rst

 * The :command:`list` command.
 * The :command:`list(APPEND)` sub-command.
 * The :command:`list() command <list>`.
 * The :command:`list(APPEND) sub-command <list>`.
 * The :variable:`CMAKE_VERSION` variable.
 * The :prop_tgt:`OUTPUT_NAME_<CONFIG>` target property.

produces:

* The `list()`_ command.
* The `list(APPEND)`_ sub-command.
* The `list() command`_.
* The `list(APPEND) sub-command`_.
* The `CMAKE_VERSION`_ variable.
* The `OUTPUT_NAME_<CONFIG>`_ target property.

Note that CMake Domain roles differ from Sphinx and reStructuredText
convention in that the form ``a<b>``, without a space preceding ``<``,
is interpreted as a name instead of link text with an explicit target.
This is necessary because we use ``<placeholders>`` frequently in
object names like ``OUTPUT_NAME_<CONFIG>``.  The form ``a <b>``,
with a space preceding ``<``, is still interpreted as a link text
with an explicit target.

.. _`list()`: https://cmake.org/cmake/help/latest/command/list.html
.. _`list(APPEND)`: https://cmake.org/cmake/help/latest/command/list.html
.. _`list(APPEND) sub-command`: https://cmake.org/cmake/help/latest/command/list.html
.. _`list() command`: https://cmake.org/cmake/help/latest/command/list.html
.. _`CMAKE_VERSION`: https://cmake.org/cmake/help/latest/variable/CMAKE_VERSION.html
.. _`OUTPUT_NAME_<CONFIG>`: https://cmake.org/cmake/help/latest/prop_tgt/OUTPUT_NAME_CONFIG.html

Style
-----

Style: Section Headers
^^^^^^^^^^^^^^^^^^^^^^

When marking section titles, make the section decoration line as long as
the title text.  Use only a line below the title, not above. For
example:

.. code-block:: rst

  Title Text
  ----------

Capitalize the first letter of each non-minor word in the title.

The section header underline character hierarchy is

* ``#``: Manual group (part) in the master document
* ``*``: Manual (chapter) title
* ``=``: Section within a manual
* ``-``: Subsection or `CMake Domain`_ object document title
* ``^``: Subsubsection or `CMake Domain`_ object document section
* ``"``: Paragraph or `CMake Domain`_ object document subsection

Style: Whitespace
^^^^^^^^^^^^^^^^^

Use two spaces for indentation.  Use two spaces between sentences in
prose.

Style: Line Length
^^^^^^^^^^^^^^^^^^

Prefer to restrict the width of lines to 75-80 columns.  This is not a
hard restriction, but writing new paragraphs wrapped at 75 columns
allows space for adding minor content without significant re-wrapping of
content.

Style: Prose
^^^^^^^^^^^^

Use American English spellings in prose.

Style: Starting Literal Blocks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Prefer to mark the start of literal blocks with ``::`` at the end of
the preceding paragraph. In cases where the following block gets
a ``code-block`` marker, put a single ``:`` at the end of the preceding
paragraph.

Style: CMake Command Signatures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Command signatures should be marked up as plain literal blocks, not as
cmake ``code-blocks``.

Signatures are separated from preceding content by a section header.
That is, use:

.. code-block:: rst

  ... preceding paragraph.

  Normal Libraries
  ^^^^^^^^^^^^^^^^

  ::

    add_library(<lib> ...)

  This signature is used for ...

Signatures of commands should wrap optional parts with square brackets,
and should mark list of optional arguments with an ellipsis (``...``).
Elements of the signature which are specified by the user should be
specified with angle brackets, and may be referred to in prose using
``inline-literal`` syntax.

Style: Boolean Constants
^^^^^^^^^^^^^^^^^^^^^^^^

Use "``OFF``" and "``ON``" for boolean values which can be modified by
the user, such as ``POSITION_INDEPENDENT_CODE``.  Such properties
may be "enabled" and "disabled". Use "``True``" and "``False``" for
inherent values which can't be modified after being set, such as the
``IMPORTED`` property of a build target.

Style: Inline Literals
^^^^^^^^^^^^^^^^^^^^^^

Mark up references to keywords in signatures, file names, and other
technical terms with ``inline-literal`` syntax, for example:

.. code-block:: rst

  If ``WIN32`` is used with :command:`add_executable`, the
  :prop_tgt:`WIN32_EXECUTABLE` target property is enabled. That command
  creates the file ``<name>.exe`` on Windows.

Style: Cross-References
^^^^^^^^^^^^^^^^^^^^^^^

Mark up linkable references as links, including repeats.
An alternative, which is used by wikipedia
(`<http://en.wikipedia.org/wiki/WP:REPEATLINK>`_),
is to link to a reference only once per article. That style is not used
in CMake documentation.

Style: Referencing CMake Concepts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If referring to a concept which corresponds to a property, and that
concept is described in a high-level manual, prefer to link to the
manual section instead of the property. For example:

.. code-block:: rst

  This command creates an :ref:`Imported Target <Imported Targets>`.

instead of:

.. code-block:: rst

  This command creates an :prop_tgt:`IMPORTED` target.

The latter should be used only when referring specifically to the
property.

References to manual sections are not automatically created by creating
a section, but code such as:

.. code-block:: rst

  .. _`Imported Targets`:

creates a suitable anchor.  Use an anchor name which matches the name
of the corresponding section.  Refer to the anchor using a
cross-reference with specified text.

Imported Targets need the ``IMPORTED`` term marked up with care in
particular because the term may refer to a command keyword, a target
property, or a concept.

Where a property, command or variable is related conceptually to others,
by for example, being related to the buildsystem description, generator
expressions or Qt, each relevant property, command or variable should
link to the primary manual, which provides high-level information.  Only
particular information relating to the command should be in the
documentation of the command.

Style: Referencing CMake Domain Objects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When referring to `CMake Domain`_ objects such as properties, variables,
commands etc, prefer to link to the target object and follow that with
the type of object it is.  For example:

.. code-block:: rst

  Set the :prop_tgt:`AUTOMOC` target property to ``ON``.

Instead of

.. code-block:: rst

  Set the target property :prop_tgt:`AUTOMOC` to ``ON``.

The ``policy`` directive is an exception, and the type us usually
referred to before the link:

.. code-block:: rst

  If policy :policy:`CMP0022` is set to ``NEW`` the behavior is ...

However, markup self-references with ``inline-literal`` syntax.
For example, within the ``add_executable`` command documentation, use

.. code-block:: rst

  ``add_executable``

not

.. code-block:: rst

  :command:`add_executable`

which is used elsewhere.

Modules
=======

The ``Modules`` directory contains CMake-language ``.cmake`` module files.

Module Documentation
--------------------

To document CMake module ``Modules/<module-name>.cmake``, modify
``Help/manual/cmake-modules.7.rst`` to reference the module in the
``toctree`` directive, in sorted order, as::

 /module/<module-name>

Then add the module document file ``Help/module/<module-name>.rst``
containing just the line::

 .. cmake-module:: ../../Modules/<module-name>.cmake

The ``cmake-module`` directive will scan the module file to extract
reStructuredText markup from comment blocks that start in ``.rst:``.
At the top of ``Modules/<module-name>.cmake``, begin with the following
license notice:

::

 # Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
 # file Copyright.txt or https://cmake.org/licensing for details.

After this notice, add a *BLANK* line.  Then, add documentation using
a `Bracket Comment`_ of the form:

::

  #[=======================================================================[.rst:
  <module-name>
  -------------

  <reStructuredText documentation of module>
  #]=======================================================================]

Any number of ``=`` may be used in the opening and closing brackets
as long as they match.  Content on the line containing the closing
bracket is excluded if and only if the line starts in ``#``.

Additional such ``.rst:`` comments may appear anywhere in the module file.
All such comments must start with ``#`` in the first column.

For example, a ``FindXxx.cmake`` module may contain:

::

  # Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
  # file Copyright.txt or https://cmake.org/licensing for details.

  #[=======================================================================[.rst:
  FindXxx
  -------

  This is a cool module.
  This module does really cool stuff.
  It can do even more than you think.

  It even needs two paragraphs to tell you about it.
  And it defines the following variables:

  ``VAR_COOL``
    this is great isn't it?
  ``VAR_REALLY_COOL``
    cool right?
  #]=======================================================================]

  <code>

  #[=======================================================================[.rst:
  .. command:: Xxx_do_something

   This command does something for Xxx::

    Xxx_do_something(some arguments)
  #]=======================================================================]
  macro(Xxx_do_something)
    <code>
  endmacro()

Test the documentation formatting by running
``cmake --help-module <module-name>``, and also by enabling the
``SPHINX_HTML`` and ``SPHINX_MAN`` options to build the documentation.
Edit the comments until generated documentation looks satisfactory.  To
have a .cmake file in this directory NOT show up in the modules
documentation, simply leave out the ``Help/module/<module-name>.rst``
file and the ``Help/manual/cmake-modules.7.rst`` toctree entry.

.. _`Bracket Comment`: https://cmake.org/cmake/help/latest/manual/cmake-language.7.html#bracket-comment

Module Functions and Macros
---------------------------

Modules may provide CMake functions and macros defined by the `function()`_
and `macro()`_ commands.  To avoid conflicts across modules, name the
functions and macros using the prefix ``<ModuleName>_`` followed by the
rest of the name, where ``<ModuleName>`` is the exact-case spelling of
the module name.  We have no convention for the portion of names after
the ``<ModuleName>_`` prefix.

For historical reasons, some modules that come with CMake do not follow
this prefix convention.  When adding new functions to these modules,
discussion during review can decide whether to follow their existing
convention or to use the module name prefix.

Documentation of public functions and macros should be provided in
the module, typically in the main `module documentation`_ at the top.
For example, a ``MyModule`` module may document a function like this::

  #[=======================================================================[.rst:
  MyModule
  --------

  This is my module.  It provides some functions.

  .. command:: MyModule_Some_Function

    This is some function:

    .. code-block:: cmake

      MyModule_Some_Function(...)
  #]=======================================================================]

Documentation may alternatively be placed just before each definition.
For example, a ``MyModule`` module may document another function like this::

  #[=======================================================================[.rst:
  .. command:: MyModule_Other_Function

    This is another function:

    .. code-block:: cmake

      MyModule_Other_Function(...)
  #]=======================================================================]
  function(MyModule_Other_Function ...)
    # ...
  endfunction()

.. _`function()`: https://cmake.org/cmake/help/latest/command/function.html
.. _`macro()`: https://cmake.org/cmake/help/latest/command/macro.html
