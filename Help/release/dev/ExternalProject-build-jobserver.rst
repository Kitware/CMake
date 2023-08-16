ExternalProject-build-jobserver
-------------------------------

* The :module:`ExternalProject` module now includes the
  ``BUILD_JOB_SERVER_AWARE`` option for the
  :command:`ExternalProject_Add` command. This option enables
  the integration of the GNU Make job server when using an
  explicit ``BUILD_COMMAND`` with certain :ref:`Makefile Generators`.
  Additionally, the :command:`ExternalProject_Add_Step` command
  has been updated to support the new ``JOB_SERVER_AWARE`` option.
