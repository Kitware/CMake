.. cmake-manual-description: CMake File-Based API

cmake-file-api(7)
*****************

.. only:: html

   .. contents::

Introduction
============

CMake provides a file-based API that clients may use to get semantic
information about the buildsystems CMake generates.  Clients may use
the API by writing query files to a specific location in a build tree
to request zero or more `Object Kinds`_.  When CMake generates the
buildsystem in that build tree it will read the query files and write
reply files for the client to read.

The file-based API uses a ``<build>/.cmake/api/`` directory at the top
of a build tree.  The API is versioned to support changes to the layout
of files within the API directory.  API file layout versioning is
orthogonal to the versioning of `Object Kinds`_ used in replies.
This version of CMake supports only one API version, `API v1`_.

API v1
======

API v1 is housed in the ``<build>/.cmake/api/v1/`` directory.
It has the following subdirectories:

``query/``
  Holds query files written by clients.
  These may be `v1 Shared Stateless Query Files`_ or
  `v1 Client Stateless Query Files`_.

``reply/``
  Holds reply files written by CMake whenever it runs to generate a build
  system.  These are indexed by a `v1 Reply Index File`_ file that may
  reference additional `v1 Reply Files`_.  CMake owns all reply files.
  Clients must never remove them.

  Clients may look for and read a reply index file at any time.
  Clients may optionally create the ``reply/`` directory at any time
  and monitor it for the appearance of a new reply index file.

v1 Shared Stateless Query Files
-------------------------------

Shared stateless query files allow clients to share requests for
major versions of the `Object Kinds`_ and get all requested versions
recognized by the CMake that runs.

Clients may create shared requests by creating empty files in the
``v1/query/`` directory.  The form is::

  <build>/.cmake/api/v1/query/<kind>-v<major>

where ``<kind>`` is one of the `Object Kinds`_, ``-v`` is literal,
and ``<major>`` is the major version number.

Files of this form are stateless shared queries not owned by any specific
client.  Once created they should not be removed without external client
coordination or human intervention.

v1 Client Stateless Query Files
-------------------------------

Client stateless query files allow clients to create owned requests for
major versions of the `Object Kinds`_ and get all requested versions
recognized by the CMake that runs.

Clients may create owned requests by creating empty files in
client-specific query subdirectories.  The form is::

  <build>/.cmake/api/v1/query/client-<client>/<kind>-v<major>

where ``client-`` is literal, ``<client>`` is a string uniquely
identifying the client, ``<kind>`` is one of the `Object Kinds`_,
``-v`` is literal, and ``<major>`` is the major version number.
Each client must choose a unique ``<client>`` identifier via its
own means.

Files of this form are stateless queries owned by the client ``<client>``.
The owning client may remove them at any time.

v1 Reply Index File
-------------------

CMake writes an ``index-*.json`` file to the ``v1/reply/`` directory
whenever it runs to generate a build system.  Clients must read the
reply index file first and may read other `v1 Reply Files`_ only by
following references.  The form of the reply index file name is::

  <build>/.cmake/api/v1/reply/index-<unspecified>.json

where ``index-`` is literal and ``<unspecified>`` is an unspecified
name selected by CMake.  Whenever a new index file is generated it
is given a new name and any old one is deleted.  During the short
time between these steps there may be multiple index files present;
the one with the largest name in lexicographic order is the current
index file.

The reply index file contains a JSON object:

.. code-block:: json

  {
    "cmake": {
      "version": {
        "major": 3, "minor": 14, "patch": 0, "suffix": "",
        "string": "3.14.0", "isDirty": false
      },
      "paths": {
        "cmake": "/prefix/bin/cmake",
        "ctest": "/prefix/bin/ctest",
        "cpack": "/prefix/bin/cpack",
        "root": "/prefix/share/cmake-3.14"
      }
    },
    "objects": [
      { "kind": "<kind>",
        "version": { "major": 1, "minor": 0 },
        "jsonFile": "<file>" },
      { "...": "..." }
    ],
    "reply": {
      "<kind>-v<major>": { "kind": "<kind>",
                           "version": { "major": 1, "minor": 0 },
                           "jsonFile": "<file>" },
      "<unknown>": { "error": "unknown query file" },
      "...": {},
      "client-<client>": {
        "<kind>-v<major>": { "kind": "<kind>",
                             "version": { "major": 1, "minor": 0 },
                             "jsonFile": "<file>" },
        "<unknown>": { "error": "unknown query file" },
        "...": {}
      }
    }
  }

The members are:

``cmake``
  A JSON object containing information about the instance of CMake that
  generated the reply.  It contains members:

  ``version``
    A JSON object specifying the version of CMake with members:

    ``major``, ``minor``, ``patch``
      Integer values specifying the major, minor, and patch version components.
    ``suffix``
      A string specifying the version suffix, if any, e.g. ``g0abc3``.
    ``string``
      A string specifying the full version in the format
      ``<major>.<minor>.<patch>[-<suffix>]``.
    ``isDirty``
      A boolean indicating whether the version was built from a version
      controlled source tree with local modifications.

  ``paths``
    A JSON object specifying paths to things that come with CMake.
    It has members for ``cmake``, ``ctest``, and ``cpack`` whose values
    are JSON strings specifying the absolute path to each tool,
    represented with forward slashes.  It also has a ``root`` member for
    the absolute path to the directory containing CMake resources like the
    ``Modules/`` directory (see :variable:`CMAKE_ROOT`).

``objects``
  A JSON array listing all versions of all `Object Kinds`_ generated
  as part of the reply.  Each array entry is a
  `v1 Reply File Reference`_.

``reply``
  A JSON object mirroring the content of the ``query/`` directory
  that CMake loaded to produce the reply.  The members are of the form

  ``<kind>-v<major>``
    A member of this form appears for each of the
    `v1 Shared Stateless Query Files`_ that CMake recognized as a
    request for object kind ``<kind>`` with major version ``<major>``.
    The value is a `v1 Reply File Reference`_ to the corresponding
    reply file for that object kind and version.

  ``<unknown>``
    A member of this form appears for each of the
    `v1 Shared Stateless Query Files`_ that CMake did not recognize.
    The value is a JSON object with a single ``error`` member
    containing a string with an error message indicating that the
    query file is unknown.

  ``client-<client>``
    A member of this form appears for each client-owned directory
    holding `v1 Client Stateless Query Files`_.
    The value is a JSON object mirroring the content of the
    ``query/client-<client>/`` directory.  The members are of the form:

    ``<kind>-v<major>``
      A member of this form appears for each of the
      `v1 Client Stateless Query Files`_ that CMake recognized as a
      request for object kind ``<kind>`` with major version ``<major>``.
      The value is a `v1 Reply File Reference`_ to the corresponding
      reply file for that object kind and version.

    ``<unknown>``
      A member of this form appears for each of the
      `v1 Client Stateless Query Files`_ that CMake did not recognize.
      The value is a JSON object with a single ``error`` member
      containing a string with an error message indicating that the
      query file is unknown.

After reading the reply index file, clients may read the other
`v1 Reply Files`_ it references.

v1 Reply File Reference
^^^^^^^^^^^^^^^^^^^^^^^

The reply index file represents each reference to another reply file
using a JSON object with members:

``kind``
  A string specifying one of the `Object Kinds`_.
``version``
  A JSON object with members ``major`` and ``minor`` specifying
  integer version components of the object kind.
``jsonFile``
  A JSON string specifying a path relative to the reply index file
  to another JSON file containing the object.

v1 Reply Files
--------------

Reply files containing specific `Object Kinds`_ are written by CMake.
The names of these files are unspecified and must not be interpreted
by clients.  Clients must first read the `v1 Reply Index File`_ and
and follow references to the names of the desired response objects.

Reply files (including the index file) will never be replaced by
files of the same name but different content.  This allows a client
to read the files concurrently with a running CMake that may generate
a new reply.  However, after generating a new reply CMake will attempt
to remove reply files from previous runs that it did not just write.
If a client attempts to read a reply file referenced by the index but
finds the file missing, that means a concurrent CMake has generated
a new reply.  The client may simply start again by reading the new
reply index file.

Object Kinds
============

The CMake file-based API reports semantic information about the build
system using the following kinds of JSON objects.  Each kind of object
is versioned independently using semantic versioning with major and
minor components.  Every kind of object has the form:

.. code-block:: json

  {
    "kind": "<kind>",
    "version": { "major": 1, "minor": 0 },
    "...": {}
  }

The ``kind`` member is a string specifying the object kind name.
The ``version`` member is a JSON object with ``major`` and ``minor``
members specifying integer components of the object kind's version.
Additional top-level members are specific to each object kind.
