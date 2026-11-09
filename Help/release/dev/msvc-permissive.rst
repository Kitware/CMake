msvc-permissive
---------------

* The :prop_tgt:`CXX_EXTENSIONS` target property is now honored by the
  ``MSVC`` compiler.  ``OFF`` adds ``/permissive-`` when the selected
  standard is permissive by default, and ``ON`` adds ``/permissive`` when
  the selected standard implies ``/permissive-``.  See policy
  :policy:`CMP0231`.
