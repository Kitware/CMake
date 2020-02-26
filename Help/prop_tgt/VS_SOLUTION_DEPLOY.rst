VS_SOLUTION_DEPLOY
------------------

Specify that the target should be marked for deployment when not targeting
Windows CE, Windows Phone or a Windows Store application.

If the target platform doesn't support deployment, this property won't have any effect.

Generator expressions are supported.

Example 1
^^^^^^^^^

This shows setting the variable for the target foo.

.. code-block:: cmake

  add_executable(foo SHARED foo.cpp)
  set_property(TARGET foo PROPERTY VS_SOLUTION_DEPLOY ON)

Example 2
^^^^^^^^^

This shows setting the variable for the Release configuration only.

.. code-block:: cmake

  add_executable(foo SHARED foo.cpp)
  set_property(TARGET foo PROPERTY VS_SOLUTION_DEPLOY "$<NOT:$<CONFIG:Release>>")
