option
------

Provide a boolean option that the user can optionally select.

.. code-block:: cmake

  option(<variable> "<help_text>" [value])

If no initial ``<value>`` is provided, boolean ``OFF`` is the default value.
If ``<variable>`` is already set as a normal or cache variable,
then the command does nothing (see policy :policy:`CMP0077`).

For options that depend on the values of other options, see
the module help for :module:`CMakeDependentOption`.

In CMake project mode, a boolean cache variable is created with the option
value. In CMake script mode, a boolean variable is set with the option value.
