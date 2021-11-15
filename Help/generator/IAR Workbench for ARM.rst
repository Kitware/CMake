IAR Workbench for ARM
-----------------

Generates IAR Workbench for ARM project and workspace files (experimental, work-in-progress).

Customizations are available through the following cache variables:

* ``IAR_COMPILER_DLIB_CONFIG``
* ``IAR_GENERAL_BUFFERED_TERMINAL_OUTPUT``
* ``IAR_GENERAL_SCANF_FORMATTER``
* ``IAR_GENERAL_PRINTF_FORMATTER``
* ``IAR_SEMIHOSTING_ENABLE``
* ``IAR_COMPILER_PATH_EXE``
* ``IAR_CPU_NAME``
* ``CMAKE_SYSTEM_NAME``
* ``IAR_DEBUGGER_CSPY_EXTRAOPTIONS``
* ``IAR_DEBUGGER_CSPY_FLASHLOADER_V3``
* ``IAR_DEBUGGER_CSPY_MACFILE``
* ``IAR_DEBUGGER_CSPY_MEMFILE``
* ``IAR_DEBUGGER_IJET_PROBECONFIG``
* ``IAR_DEBUGGER_PROBE``
* ``IAR_DEBUGGER_LOGFILE``
* ``IAR_LINKER_ENTRY_ROUTINE``
* ``IAR_LINKER_ICF_FILE``
* ``IAR_TARGET_ARCHITECTURE``
* ``IAR_TARGET_RTOS``
* ``IAR_COMPILER_PREINCLUDE``
* ``IAR_WORKBENCH_VERSION``


.. note::
  This generator is deemed experimental as of CMake |release|
  and is still a work in progress.  Future versions of CMake
  may make breaking changes as the generator matures.
