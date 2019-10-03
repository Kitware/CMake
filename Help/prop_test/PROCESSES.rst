PROCESSES
----------

Set to specify the number of processes spawned by a test, and the resources
that they require. See :ref:`hardware allocation <ctest-hardware-allocation>`
for more information on how this property integrates into the CTest hardware
allocation feature.

The ``PROCESSES`` property is a :ref:`semicolon-separated list <CMake Language
Lists>` of process descriptions. Each process description consists of an
optional number of processes for the description followed by a series of
resource requirements for those processes. These requirements (and the number
of processes) are separated by commas. The resource requirements consist of the
name of a resource type, followed by a colon, followed by an unsigned integer
specifying the number of slots required on one resource of the given type.

Please note that these processes are not spawned by CTest. The ``PROCESSES``
property merely tells CTest what processes the test expects to launch. It is up
to the test itself to do this process spawning, and read the :ref:`environment
variables <ctest-hardware-environment-variables>` to determine which resources
each process has been allocated.

Consider the following example:

.. code-block:: cmake

  add_test(NAME MyTest COMMAND MyExe)
  set_property(TEST MyTest PROPERTY PROCESSES
    "2,gpus:2"
    "gpus:4,crypto_chips:2")

In this example, there are two process descriptions (implicitly separated by a
semicolon.) The content of the first description is ``2,gpus:2``. This
description spawns 2 processes, each of which requires 2 slots from a single
GPU. The content of the second description is ``gpus:4,crypto_chips:2``. This
description does not specify a process count, so a default of 1 is assumed.
This single process requires 4 slots from a single GPU and 2 slots from a
single cryptography chip. In total, 3 processes are spawned from this test,
each with their own unique requirements.

When CTest sets the :ref:`environment variables
<ctest-hardware-environment-variables>` for a test, it assigns a process number
based on the process description, starting at 0 on the left and the number of
processes minus 1 on the right. For example, in the example above, the two
processes in the first description would have IDs of 0 and 1, and the single
process in the second description would have an ID of 2.

Both the ``PROCESSES`` and :prop_test:`RESOURCE_LOCK` properties serve similar
purposes, but they are distinct and orthogonal. Resources specified by
``PROCESSES`` do not affect :prop_test:`RESOURCE_LOCK`, and vice versa. Whereas
:prop_test:`RESOURCE_LOCK` is a simpler property that is used for locking one
global resource, ``PROCESSES`` is a more advanced property that allows multiple
tests to simultaneously use multiple resources of the same type, specifying
their requirements in a fine-grained manner.
