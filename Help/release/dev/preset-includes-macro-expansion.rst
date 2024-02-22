preset-includes-macro-expansion
-------------------------------

* :manual:`cmake-presets(7)` files now support schema version ``9``:

  * ``include`` fields now expand all macros except ``$env{}`` and
    preset-specific macros.
