set_tests_properties
--------------------

Set a property of the tests.

.. code-block:: cmake

  set_tests_properties(test1 [test2...] PROPERTIES prop1 value1 prop2 value2)

Sets a property for the tests.  If the test is not found, CMake
will report an error.

Test property values may be specified using
:manual:`generator expressions <cmake-generator-expressions(7)>`
for tests created by the :command:`add_test(NAME)` signature.

See Also
^^^^^^^^

* :command:`add_test`
* :command:`define_property`
* the more general :command:`set_property` command
* :ref:`Target Properties` for the list of properties known to CMake
