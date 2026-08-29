cmake_host_system_information-distrib-host
------------------------------------------

* The :command:`cmake_host_system_information` command's ``DISTRIB_*`` queries
  now read the host os-release by default.  See policy :policy:`CMP0221`.
  A new ``FROM_SYSROOT <bool>`` option explicitly enables or disables reading
  the target os-release from :variable:`CMAKE_SYSROOT`.
