FIXTURE_REPEAT_MODE
-------------------

.. versionadded:: 4.5

Specifies how a test fixture behaves when :manual:`ctest(1)` repeats tests
with its :ctest-option:`--repeat` option.

The property describes the fixture rather than the test carrying it, so
setting it on any one of the fixture's setup or cleanup tests is enough.  It
has no effect on a test that has neither a :prop_test:`FIXTURES_SETUP` nor a
:prop_test:`FIXTURES_CLEANUP` property, although :manual:`ctest(1)` still
rejects a value it does not recognize.

The value must be one of the following:

``AROUND_ALL_REPEATS``
  The fixture runs once, around all repetitions of the tests requiring it::

    setup -> test -> test -> test -> cleanup

``AROUND_EACH_REPEAT``
  The fixture repeats with the tests requiring it, so that every repetition
  of a test gets a fresh setup and its own cleanup::

    setup -> test -> cleanup -> setup -> test -> cleanup -> ...

``EACH_TEST_SEPARATELY``
  No test is treated specially: every test, including the fixture's own setup
  and cleanup tests, runs all of its repetitions before the next test starts::

    setup -> setup -> setup -> test -> test -> test -> cleanup -> ...

  This is the behavior of CMake 4.4 and below.

If the property is not set on any of a fixture's setup or cleanup tests, the
behavior is determined by policy :policy:`CMP0224`.

Fixtures that repeat together must agree on the mode.  All of a fixture's own
setup and cleanup tests must request the same mode, and so must any two
fixtures that share a test, since one test cannot repeat with one fixture and
not with another it takes part in.  :manual:`ctest(1)` reports an error and
runs nothing if they disagree.

In ``AROUND_EACH_REPEAT`` mode the fixture's tests repeat as a unit, so
:ctest-option:`--repeat` applies its condition to the unit as a whole.
``until-fail`` repeats while every test in the unit passes, ``until-pass``
repeats while any of them does not pass, and ``after-timeout`` repeats while
any of them times out.  A test that requires two fixtures in
``AROUND_EACH_REPEAT`` mode makes those fixtures repeat together.

Only the last repetition of the unit is reported, as for a test repeating on
its own, and a test that lists one of the unit's tests in its
:prop_test:`DEPENDS` property runs after that last repetition.  If
:manual:`ctest(1)` is interrupted part-way through the repetitions,
:ctest-option:`-F` resumes by running the whole unit again.

Example
^^^^^^^

.. code-block:: cmake

  add_test(NAME start_server COMMAND start_server)
  set_tests_properties(start_server PROPERTIES
    FIXTURES_SETUP Server
    FIXTURE_REPEAT_MODE AROUND_EACH_REPEAT)

  add_test(NAME query_server COMMAND query_server)
  set_tests_properties(query_server PROPERTIES FIXTURES_REQUIRED Server)

  add_test(NAME stop_server COMMAND stop_server)
  set_tests_properties(stop_server PROPERTIES FIXTURES_CLEANUP Server)

With ``ctest --repeat until-fail:3``, this runs a fresh server for each
attempt at ``query_server``::

  start_server -> query_server -> stop_server
  start_server -> query_server -> stop_server
  start_server -> query_server -> stop_server

See Also
^^^^^^^^

* :prop_test:`FIXTURES_SETUP`
* :prop_test:`FIXTURES_CLEANUP`
* :prop_test:`FIXTURES_REQUIRED`
* :policy:`CMP0224`
