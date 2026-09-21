USES_TERMINAL
-------------

.. versionadded:: 4.5

``USES_TERMINAL`` is a boolean which request that the commands produced by the
instantiation of the rule will be given direct access to the terminal if
possible. With the :ref:`Ninja Generators`, this places the command in the
``console`` :prop_gbl:`pool <JOB_POOLS>`.
