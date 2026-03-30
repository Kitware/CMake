Rust_EDITION
------------

.. versionadded:: 4.4

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_RUST``.

The Rust edition required to build this target. It is initialized from the
:variable:`CMAKE_Rust_EDITION` variable if defined. Setting this property
results in adding a flag such as ``--edition=2018`` to the compile line.

Supported values are:

``2015``
  Rust 2015, the default edition, started with Rust 1.0.

``2018``
  Rust 2018, available since Rust 1.31.

``2021``
  Rust 2021, available since Rust 1.56.

``2024``
  Rust 2024, available since Rust 1.85.
