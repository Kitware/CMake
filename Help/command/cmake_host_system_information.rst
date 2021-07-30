cmake_host_system_information
-----------------------------

Query host system specific information.

.. code-block:: cmake

  cmake_host_system_information(RESULT <variable> QUERY <key> ...)

Queries system information of the host system on which cmake runs.
One or more ``<key>`` can be provided to select the information to be
queried.  The list of queried values is stored in ``<variable>``.

``<key>`` can be one of the following values:

``NUMBER_OF_LOGICAL_CORES``
  Number of logical cores

``NUMBER_OF_PHYSICAL_CORES``
  Number of physical cores

``HOSTNAME``
  Hostname

``FQDN``
  Fully qualified domain name

``TOTAL_VIRTUAL_MEMORY``
  Total virtual memory in MiB [#mebibytes]_

``AVAILABLE_VIRTUAL_MEMORY``
  Available virtual memory in MiB [#mebibytes]_

``TOTAL_PHYSICAL_MEMORY``
  Total physical memory in MiB [#mebibytes]_

``AVAILABLE_PHYSICAL_MEMORY``
  Available physical memory in MiB [#mebibytes]_

``IS_64BIT``
  .. versionadded:: 3.10

  One if processor is 64Bit

``HAS_FPU``
  .. versionadded:: 3.10

  One if processor has floating point unit

``HAS_MMX``
  .. versionadded:: 3.10

  One if processor supports MMX instructions

``HAS_MMX_PLUS``
  .. versionadded:: 3.10

  One if processor supports Ext. MMX instructions

``HAS_SSE``
  .. versionadded:: 3.10

  One if processor supports SSE instructions

``HAS_SSE2``
  .. versionadded:: 3.10

  One if processor supports SSE2 instructions

``HAS_SSE_FP``
  .. versionadded:: 3.10

  One if processor supports SSE FP instructions

``HAS_SSE_MMX``
  .. versionadded:: 3.10

  One if processor supports SSE MMX instructions

``HAS_AMD_3DNOW``
  .. versionadded:: 3.10

  One if processor supports 3DNow instructions

``HAS_AMD_3DNOW_PLUS``
  .. versionadded:: 3.10

  One if processor supports 3DNow+ instructions

``HAS_IA64``
  .. versionadded:: 3.10

  One if IA64 processor emulating x86

``HAS_SERIAL_NUMBER``
  .. versionadded:: 3.10

  One if processor has serial number

``PROCESSOR_SERIAL_NUMBER``
  .. versionadded:: 3.10

  Processor serial number

``PROCESSOR_NAME``
  .. versionadded:: 3.10

  Human readable processor name

``PROCESSOR_DESCRIPTION``
  .. versionadded:: 3.10

  Human readable full processor description

``OS_NAME``
  .. versionadded:: 3.10

  See :variable:`CMAKE_HOST_SYSTEM_NAME`

``OS_RELEASE``
  .. versionadded:: 3.10

  The OS sub-type e.g. on Windows ``Professional``

``OS_VERSION``
  .. versionadded:: 3.10

  The OS build ID

``OS_PLATFORM``
  .. versionadded:: 3.10

  See :variable:`CMAKE_HOST_SYSTEM_PROCESSOR`

For Linux distributions additional ``<key>`` values are available to get operating
system identification as described in the `man 5 os-release`_.

``DISTRIB_INFO``
  .. versionadded:: 3.22

  Read :file:`/etc/os-release` file and define the given ``<variable>``
  into a list of read variables

``DISTRIB_<name>``
  .. versionadded:: 3.22

  Get the ``<name>`` variable if it exists in the :file:`/etc/os-release` file

  Example:

  .. code-block:: cmake

      cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
      message(STATUS "${PRETTY_NAME}")

      cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)

      foreach(VAR IN LISTS DISTRO)
        message(STATUS "${VAR}=`${${VAR}}`")
      endforeach()


  Output::

    -- Ubuntu 20.04.2 LTS
    -- DISTRO_BUG_REPORT_URL=`https://bugs.launchpad.net/ubuntu/`
    -- DISTRO_HOME_URL=`https://www.ubuntu.com/`
    -- DISTRO_ID=`ubuntu`
    -- DISTRO_ID_LIKE=`debian`
    -- DISTRO_NAME=`Ubuntu`
    -- DISTRO_PRETTY_NAME=`Ubuntu 20.04.2 LTS`
    -- DISTRO_PRIVACY_POLICY_URL=`https://www.ubuntu.com/legal/terms-and-policies/privacy-policy`
    -- DISTRO_SUPPORT_URL=`https://help.ubuntu.com/`
    -- DISTRO_UBUNTU_CODENAME=`focal`
    -- DISTRO_VERSION=`20.04.2 LTS (Focal Fossa)`
    -- DISTRO_VERSION_CODENAME=`focal`
    -- DISTRO_VERSION_ID=`20.04`

.. rubric:: Footnotes

.. [#mebibytes] One MiB (mebibyte) is equal to 1024x1024 bytes.

.. _man 5 os-release: https://www.freedesktop.org/software/systemd/man/os-release.html
