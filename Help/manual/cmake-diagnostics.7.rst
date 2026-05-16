.. cmake-manual-description: CMake Diagnostics Reference

cmake-diagnostics(7)
********************

.. only:: html

   .. contents::

.. _cmake-diagnostics-intro:

Introduction
============

.. versionadded:: 4.4

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

Controlling Diagnostics
=======================

Each diagnostic category has an associated action to be taken when that
diagnostic is triggered.  Most categories will warn by default.  The available
actions are described in the :command:`cmake_diagnostic` command documentation.

CMake maintains a diagnostic state stack that is similar to the policy state.
The initial state of the stack is determined by four factors, which are, in
order of precedence:

* The default action associated with the diagnostic.

* The action associated with the diagnostic stored in the CMake variable cache,
  which is used to persist the initial state between CMake runs.

* The :preset:`configurePresets.warnings` and :preset:`configurePresets.errors`
  fields of :manual:`CMake Presets <cmake-presets(7)>`.

* The :option:`-W[no-][error=] <cmake -W>` command line arguments.

.. note::

  Because command line arguments operate both recursively and in the order
  specified, some combinations of diagnostic arguments may result in later
  arguments completely overwriting the action of earlier arguments.  For
  example, ``-Wno-child -Wparent`` will result in the ``child`` warning being
  enabled, because ``-Wparent`` promotes both ``parent`` and ``child`` to at
  least ``WARN`` severity.  CMake presets are evaluated in order from most
  ancestral to least ancestral.

During script execution, the :command:`cmake_diagnostic` command can be used to
query or alter the state, or to perform limited stack manipulations.

When a diagnostic is issued at configure time (or during script execution, when
CMake is running in script mode), the current diagnostic state controls the
action.  Diagnostics issued at generate time, or outside of the configuration
and generation phases must make use of recorded state information. While CMake
strives to preserve this information in a way that matches the recorded state
to the state as of the CMake command which ultimately causes a diagnostic to be
issued, CMake may sometimes fall back to the state when processing of a
subdirectory completed, or even the root state.  This may limit the ability of
the :command:`cmake_diagnostic` command to control such diagnostics, especially
if called from a function or included file.  This is especially the case for
diagnostics that are not directly coupled to a CMake command.

Diagnostic Categories
=====================

The following categories are defined:

.. toctree::
   :maxdepth: 1

   /diagnostic/CMD_AUTHOR
   /diagnostic/CMD_DEPRECATED
   /diagnostic/CMD_EXPERIMENTAL
   /diagnostic/CMD_INSTALL_ABSOLUTE_DESTINATION
   /diagnostic/CMD_POLICY
   /diagnostic/CMD_UNINITIALIZED
   /diagnostic/CMD_UNUSED_CLI
