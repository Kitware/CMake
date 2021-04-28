ARMClang-cpu-arch-flags
-----------------------

* ``ARMClang`` cpu/arch compile and link flags are no longer added
  automatically based on the :variable:`CMAKE_SYSTEM_PROCESSOR`
  variable or the undocumented ``CMAKE_SYSTEM_ARCH`` variable.
  They must be specified explicitly.  See policy :policy:`CMP0123`.
