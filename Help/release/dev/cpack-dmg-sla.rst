cpack-dmg-sla
-------------

* The :cpack_gen:`CPack DragNDrop Generator` no longer attaches
  :variable:`CPACK_RESOURCE_FILE_LICENSE` as the license agreement in
  the generated ``.dmg`` unless explicitly activated by a
  :variable:`CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE` option.
  In CMake projects, the :module:`CPack` module enables the option
  by default for compatibility.
