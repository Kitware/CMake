cpack-dmg-rtf-for-sla
---------------------

* The :cpack_gen:`CPack DragNDrop Generator` learned to handle
  RTF formatted license files.  When :variable:`CPACK_DMG_SLA_DIR`
  variable is set, <language>.license.rtf is considered, but
  only as a fallback when the plaintext (.txt) file is not found
  in order to maintain backwards compatibility.
