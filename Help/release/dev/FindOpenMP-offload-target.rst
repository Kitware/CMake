FindOpenMP-offload-target
-------------------------

* The :module:`FindOpenMP` module now provides ``OpenMP::OpenMP_<lang>_Offload``
  imported targets for code whose OpenMP ``target`` regions may be offloaded
  to a device.  Offload is enabled with the new ``OpenMP_<lang>_OFFLOAD`` and
  ``OpenMP_<lang>_OFFLOAD_FLAGS`` variables.
