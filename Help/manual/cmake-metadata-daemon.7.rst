.. cmake-manual-description: CMake Metadata Daemon

cmake-metadata-daemon(7)
************************

.. only:: html

   .. contents::

Introduction
============

:manual:`cmake(1)` is capable of providing semantic information about
CMake code it executes to generate a buildsystem.  If executed with
the ``-E daemon $buildDir`` command line options, it starts in a long
running mode and allows a client to request the available information via
a JSON protocol.

The protocol is designed to be useful to IDEs, refactoring tools, and
other tools which have a need to understand the buildsystem in entirity.

A single :manual:`cmake-buildsystem(7)` may describe buildsystem contents
and build properties which differ based on
:manual:`generation-time context <cmake-generator-expressions(7)>`
including:

* The Platform (eg, Windows, APPLE, Linux).
* The build configuration (eg, Debug, Release, Coverage).
* The Compiler (eg, MSVC, GCC, Clang) and compiler version.
* The language of the source files compiled.
* Available compile features (eg CXX variadic templates).
* CMake policies.

The protocol aims to provide information to tooling to satisfy several
needs:

#. Provide a complete and easily parsed source of all information relevant
   to the tooling as it relates to the source code.  There should be no need
   for tooling to parse generated buildsystems to access include directories
   or compile defintions for example.
#. Semantic information about the CMake buildsystem itself.
#. Provide a stable interface for reading the information in the CMake cache.
#. Information for determining when cmake needs to be re-run as a result of
   file changes.


Operation
---------

Start :manual:`cmake(1)` in the daemon command mode, supplying the path to
the build directory to process::

  cmake -E daemon /path/to/build

Messages sent to and from the process are wrapped in magic strings.

When the process is running it will write a json message to stdout::

  [== CMake MetaMagic ==[
  {
   "progress":"process-started"
  }
  ]== CMake MetaMagic ==]

At this point the client may write on the stdin of the process to complete
the handshake. The client requests the protocol version to use::

  [== CMake MetaMagic ==[
  {
    "type":"handshake",
    "protocolVersion":"3.5"
  }
  ]== CMake MetaMagic ==]

Doing so triggers the build dir to be configured and computed::

  [== CMake MetaMagic ==[
  {
    "progress":"initialized"
  }
  ]== CMake MetaMagic ==]

  [== CMake MetaMagic ==[
  {
    "progress":"configured"
  }
  ]== CMake MetaMagic ==]

  [== CMake MetaMagic ==[
  {
    "progress":"computed"
  }
  ]== CMake MetaMagic ==]

When it is done, the server tells the client the source
directory and the main project name::

  [== CMake MetaMagic ==[
  {
   "binary_dir":"/path/to/build",
   "progress":"idle",
   "project_name":"my-project",
   "source_dir":"/path/to/source"
  }
  ]== CMake MetaMagic ==]

The daemon is now ready to accept further requests.


Protocol API
------------

version
^^^^^^^

Request::

  [== CMake MetaMagic ==[
  {
    "type":"version"
  }
  ]== CMake MetaMagic ==]

Response::

  [== CMake MetaMagic ==[
  {
    "version":"3.5.0"
  }
  ]== CMake MetaMagic ==]
