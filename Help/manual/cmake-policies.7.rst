.. cmake-manual-description: CMake Policies Reference

cmake-policies(7)
*****************

.. only:: html or latex

   .. contents::

Introduction
============

Policies in CMake are used to preserve backward compatible behavior
across multiple releases.  When a new policy is introduced, newer CMake
versions will begin to warn about the backward compatible behavior.  It
is possible to disable the warning by explicitly requesting the OLD, or
backward compatible behavior using the :command:`cmake_policy` command.
It is also possible to request NEW, or non-backward compatible behavior
for a policy, also avoiding the warning.

The :command:`cmake_minimum_required` command does more than report an
error if a too-old version of CMake is used to build a project.  It
also sets all policies introduced in that CMake version or earlier to
NEW behavior.

The :variable:`CMAKE_MINIMUM_REQUIRED_VERSION` variable may also be used
to determine whether to report an error on use of deprecated macros or
functions.

All Policies
============

.. toctree::
   :maxdepth: 1

   /policy/CMP0000
   /policy/CMP0001
   /policy/CMP0002
   /policy/CMP0003
   /policy/CMP0004
   /policy/CMP0005
   /policy/CMP0006
   /policy/CMP0007
   /policy/CMP0008
   /policy/CMP0009
   /policy/CMP0010
   /policy/CMP0011
   /policy/CMP0012
   /policy/CMP0013
   /policy/CMP0014
   /policy/CMP0015
   /policy/CMP0016
   /policy/CMP0017
   /policy/CMP0018
   /policy/CMP0019
   /policy/CMP0020
   /policy/CMP0021
   /policy/CMP0022
   /policy/CMP0023
   /policy/CMP0024
   /policy/CMP0025
   /policy/CMP0026
   /policy/CMP0027
   /policy/CMP0028
   /policy/CMP0029
   /policy/CMP0030
   /policy/CMP0031
   /policy/CMP0032
   /policy/CMP0033
   /policy/CMP0034
   /policy/CMP0035
   /policy/CMP0036
