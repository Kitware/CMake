.. cmake-manual-description: CMake Diagnostics Reference

cmake-diagnostics(7)
********************

.. only:: html

   .. contents::

.. _cmake-diagnostics-intro:

Introduction
============

CMake Diagnostics are the mechanism by which CMake categorizes and presents
certain advisory information about a project's configuration and the generation
of its build system.  These diagnostics can be seen as the build system
equivalent of compiler warnings.  Diagnostics provide feedback on potential
issues in several categories:

* Issues that may impact the success of the build.

* Issues that may impact the correctness of the build.

* Issues that may impact the correctness of the project packaging.

* Issues that may impact the ability of the project
  to be built with newer versions of dependencies.

* Issues that may impact the ability of the project
  to be built with newer versions of CMake.

Diagnostic Actions
------------------

The action taken when a particular diagnostic is triggered depends on the
diagnostic category.  Most categories will warn by default.  The
:command:`cmake_diagnostic` command and :option:`-W <cmake -W>` options can be
used to control what action occurs when a diagnostic of a particular category
is triggered.  The possible actions are described in the documentation of the
same.

Diagnostic Categories
=====================

The following categories are defined.

``CMD_AUTHOR`` (``-Wauthor``)
-----------------------------

:Default: Warn

Warn about a build system's incorrect use of CMake, or of a CMake interface
provided by a dependency.  This is the category triggered by
:command:`message(AUTHOR_WARNING)`.  It is also the ancestor of many other
diagnostic categories.

The most important aspect of this category is that it represents issues with
a project's build system which typically require alteration to the same.  This
is to say that users simply trying to build a project obtained elsewhere will
typically not be interested in these warnings, except to perhaps report them
to the project's developer(s).

``CMD_DEPRECATED`` (``-Wdeprecated``)
-------------------------------------

:Default: Warn
:Parent: ``CMD_AUTHOR``

Warn about use of a deprecated function or package.  This is the category
triggered by :command:`message(DEPRECATION)`.

``CMD_UNINITIALIZED`` (``-Wuninitialized``)
-------------------------------------------

:Default: Ignore

Warn if an uninitialized variable is dereferenced.

``CMD_UNUSED_CLI`` (``-Wunused-cli``)
-------------------------------------

:Default: Warn

Warn about variables that are declared on the command line, but not used.

Although the action of this warning category can be queried as usual, changes
made using the :command:`cmake_diagnostic` command have no effect.
