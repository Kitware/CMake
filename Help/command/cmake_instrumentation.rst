cmake_instrumentation
---------------------

.. versionadded:: 4.0

.. note::

   This command is only available when experimental support for instrumentation
   has been enabled by the ``CMAKE_EXPERIMENTAL_INSTRUMENTATION`` gate.

Enables interacting with the
:manual:`CMake Instrumentation API <cmake-instrumentation(7)>`.

This allows for configuring instrumentation at the project-level.

.. code-block:: cmake

  cmake_instrumentation(
    API_VERSION <version>
    DATA_VERSION <version>
    [HOOKS <hooks>...]
    [QUERIES <queries>...]
    [CALLBACK <callback>]
  )

The ``API_VERSION`` and ``DATA_VERSION`` must always be given.  Currently, the
only supported value for both fields is 1.  See :ref:`cmake-instrumentation API v1`
for details of the ``API_VERSION`` and :ref:`cmake-instrumentation Data v1` for details
of the ``DATA_VERSION``.

Each of the optional keywords ``HOOKS``, ``QUERIES``, and ``CALLBACK``
correspond to one of the parameters to the :ref:`cmake-instrumentation v1 Query Files`.
The ``CALLBACK`` keyword can be provided multiple times to create multiple callbacks.

Whenever ``cmake_instrumentation`` is invoked, a query file is generated in
``<build>/.cmake/instrumentation/v1/query/generated`` to enable instrumentation
with the provided arguments.

Example
^^^^^^^

The following example shows an invocation of the command and its
equivalent JSON query file.

.. code-block:: cmake

  cmake_instrumentation(
    API_VERSION 1
    DATA_VERSION 1
    HOOKS postGenerate preCMakeBuild postCMakeBuild
    QUERIES staticSystemInformation dynamicSystemInformation
    CALLBACK ${CMAKE_COMMAND} -P /path/to/handle_data.cmake
    CALLBACK ${CMAKE_COMMAND} -P /path/to/handle_data_2.cmake
  )

.. code-block:: json

  {
    "version": 1,
    "hooks": [
      "postGenerate", "preCMakeBuild", "postCMakeBuild"
    ],
    "queries": [
      "staticSystemInformation", "dynamicSystemInformation"
    ],
    "callbacks": [
      "/path/to/cmake -P /path/to/handle_data.cmake"
      "/path/to/cmake -P /path/to/handle_data_2.cmake"
    ]
  }
