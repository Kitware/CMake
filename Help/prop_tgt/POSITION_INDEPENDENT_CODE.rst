POSITION_INDEPENDENT_CODE
-------------------------

Whether to create a position-independent target

The ``POSITION_INDEPENDENT_CODE`` property determines whether position
independent executables or libraries will be created.  This
property is ``True`` by default for ``SHARED`` and ``MODULE`` library
targets.  For other targets, this property is initialized by the value
of the :variable:`CMAKE_POSITION_INDEPENDENT_CODE` variable if it is set
when the target is created, or ``False`` otherwise.

.. note::

  For executable targets, the link step is controlled by the :policy:`CMP0083`
  policy and the :module:`CheckPIESupported` module.
