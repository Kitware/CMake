SBOM
----

* The experimental gate for Software Bill of Materials (SBOM) generation
  has been dropped. Projects can generate SBOMs with the :command:`export(SBOM)`
  and :command:`install(SBOM)` commands. SBOMs can be generated automatically
  via :variable:`CMAKE_INSTALL_SBOM_FORMATS`.
