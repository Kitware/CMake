reorder-sys-includes
--------------------

* Include directories marked as ``SYSTEM`` are now moved after non-system
  directories.  The ``-isystem`` flag does this automatically, so moving
  them explicitly to the end makes the behavior consistent on compilers
  that do not have any ``-isystem`` flag.
