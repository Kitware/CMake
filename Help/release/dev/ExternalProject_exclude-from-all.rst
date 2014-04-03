ExternalProject_exclude-from-all
--------------------------------

* The :module:`ExternalProject` module ``ExternalProject_Add`` command
  learned a new ``EXCLUDE_FROM_ALL`` option to cause the external
  project target to have the :prop_tgt:`EXCLUDE_FROM_ALL` target
  property set.

* The :module:`ExternalProject` module ``ExternalProject_Add_Step`` command
  learned a new ``EXCLUDE_FROM_MAIN`` option to cause the step to not be
  a direct dependency of the main external project target.
