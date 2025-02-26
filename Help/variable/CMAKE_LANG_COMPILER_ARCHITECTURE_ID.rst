CMAKE_<LANG>_COMPILER_ARCHITECTURE_ID
-------------------------------------

.. versionadded:: 3.10

Identifier indicating the
target architecture of the compiler for language ``<LANG>``.

Possible values for each platform are documented in the following sections.

Windows Platforms with MSVC ABI
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. versionadded:: 3.10

These identifiers are used when the :variable:`CMAKE_<LANG>_COMPILER`
targets Windows with a MSVC ABI (``_WIN32`` and ``_MSC_VER`` are defined).

``ARM64``
  ARM 64-bit

``ARM64EC``
  ARM 64-bit Emulation-Compatible

``ARMV4I``, ``ARMV5I``, ``ARMV7``
  ARM 32-bit

``IA64``
  Itanium 64-bit

``MIPS``
  MIPS

``SHx``, ``SH3``, ``SH3DSP``, ``SH4``, ``SH5``
  SuperH

``x64``
  Intel 64-bit

``X86``
  Intel 32-bit

Windows Platforms with Watcom ABI
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. versionadded:: 3.10

These identifiers are used when :variable:`CMAKE_<LANG>_COMPILER_ID` is
``OpenWatcom`` or ``Watcom``.

``I86``
  Intel 16-bit

``X86``
  Intel 32-bit

Green Hills MULTI Platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. versionadded:: 3.14

These identifiers are used when :variable:`CMAKE_<LANG>_COMPILER_ID` is
``GHS``.

``ARM``
  ARM 32-bit

``PPC``
  PowerPC 32-bit

``PPC64``
  PowerPC 64-bit

``x64``
  Intel 64-bit

``X86``
  Intel 32-bit

IAR Platforms
^^^^^^^^^^^^^

.. versionadded:: 3.10

These identifiers are used when :variable:`CMAKE_<LANG>_COMPILER_ID` is
``IAR``.

``8051``
  ..

``ARM``
  ..

``AVR``
  ..

``MSP430``
  ..

``RH850``
  ..

``RISCV``
  ..

``RL78``
  ..

``RX``
  ..

``STM8``
  ..

``V850``
  ..

TASKING Platforms
^^^^^^^^^^^^^^^^^

.. versionadded:: 3.25

These identifiers are used when :variable:`CMAKE_<LANG>_COMPILER_ID` is
``Tasking``.

``8051``
  ..

``ARC``
  ..

``ARM``
  ..

``MCS``
  ..

``PCP``
  ..

``TriCore``
  ..

Texas Instruments Platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. versionadded:: 3.19

These identifiers are used when :variable:`CMAKE_<LANG>_COMPILER_ID` is
``TI``.

``ARM``
  ..

``Blackfin``
  ..

``MSP430``
  ..

``SHARC``
  ..

``TMS320C28x``
  ..

``TMS320C6x``
  ..
