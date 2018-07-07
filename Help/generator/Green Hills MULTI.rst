Green Hills MULTI
-----------------

Generates Green Hills MULTI project files (experimental, work-in-progress).

Customizations that are used to pick toolset and target system:

The ``-A <arch>`` can be supplied for setting the target architecture.
``<arch>`` usually is one of "arm", "ppc", "86", etcetera.  If the target architecture
is not specified then the default architecture of "arm" will be used.

Customizations are available through the following cache variables:

* ``GHS_BSP_NAME``
* ``GHS_CUSTOMIZATION``
* ``GHS_GPJ_MACROS``
* ``GHS_OS_DIR``

.. note::
  This generator is deemed experimental as of CMake |release|
  and is still a work in progress.  Future versions of CMake
  may make breaking changes as the generator matures.
