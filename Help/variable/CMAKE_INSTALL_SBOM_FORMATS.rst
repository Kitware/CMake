CMAKE_INSTALL_SBOM_FORMATS
--------------------------

.. versionadded:: 4.5

.. note::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_GENERATE_SBOM``.

Enable automatic generation and installation of Software Bill of Materials
(SBOM) documents for the project's install export sets.

Set this variable in the top-level directory, or in the cache, to ``spdx``
to generate an SBOM for each export set created by :command:`install(TARGETS)`
with the ``EXPORT`` option.  Export sets already covered by an explicit
:command:`install(SBOM)` call are skipped.

Each automatically generated SBOM uses the export set name as its package
name and the top-level project name to obtain project metadata.  It is
installed under ``<libdir>/sbom/<export-name>``, where ``<libdir>`` is the
value of ``CMAKE_INSTALL_LIBDIR``, or ``lib`` if that variable is unset or
empty.

The value uses the same format names as the ``FORMAT`` option of
:command:`install(SBOM)` and :command:`export(SBOM)`.  Currently, ``spdx`` and
``spdx-3.0+json`` are supported and select SPDX JSON output.

For example, a package distributor can enable SBOM installation without
modifying a project's build scripts:

.. code-block:: console

  cmake -S . -B build -DCMAKE_INSTALL_SBOM_FORMATS=spdx
  cmake --build build
  cmake --install build

Projects that need to customize SBOM metadata, format, or destination should
use :command:`install(SBOM)` directly.
