fileapi-direct-dependencies
---------------------------

* The :manual:`cmake-file-api(7)` "codemodel" version 2 now includes imported
  targets and all interface library targets in its replies.  Previously,
  imported targets were omitted, and only those interface targets that
  participated in the build system were included.  The following changes
  support these new additions:

  * The "target" object gained ``imported``, ``local``, and ``abstract`` fields.
  * The "target" object's ``type`` field can now also hold the value
    ``UNKNOWN_LIBRARY``.
  * The "codemodel" object's ``configurations`` entries gained a new
    ``abstractTargets`` array.
  * Entries in the ``directories`` and ``projects`` arrays of the "codemodel"
    object's ``configurations`` entries gained a new ``abstractTargetIndexes``
    array.
