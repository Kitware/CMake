CMake Tutorial
**************

Introduction
============

The CMake tutorial provides a step-by-step guide that covers common build
system issues that CMake helps address. Seeing how various topics all
work together in an example project can be very helpful.

Steps
=====

.. include:: include/source.rst

|tutorial_source|
Each step has its own subdirectory containing code that may be used as a
starting point. The tutorial examples are progressive so that each step
provides the complete solution for the previous step.

.. toctree::
  :maxdepth: 2

  Before You Begin
  Getting Started with CMake
  CMake Language Fundamentals
  Configuration and Cache Variables
  In-Depth CMake Target Commands
  In-Depth CMake Library Concepts
  In-Depth System Introspection
  Custom Commands and Generated Files
  Testing and CTest
  Installation Commands and Concepts
  Finding Dependencies
  Miscellaneous Features

..
  Whenever a step above is renamed or removed, leave forwarding text in
  its original document file, and list it below to preserve old links
  to cmake.org/cmake/help/latest/ URLs.

.. toctree::
  :maxdepth: 1
  :hidden:

  A Basic Starting Point
  Adding a Library
  Adding Usage Requirements for a Library
  Adding Generator Expressions
  Installing and Testing
  Adding Support for a Testing Dashboard
  Adding System Introspection
  Adding a Custom Command and Generated File
  Packaging an Installer
  Selecting Static or Shared Libraries
  Adding Export Configuration
  Packaging Debug and Release
