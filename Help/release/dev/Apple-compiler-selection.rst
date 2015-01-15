Apple-compiler-selection
------------------------

* On OS X with Makefile and Ninja generators, when a compiler is found
  in ``/usr/bin`` it is now mapped to the corresponding compiler inside
  the Xcode application folder, if any.  This allows such build
  trees to continue to work with their original compiler even when
  ``xcode-select`` switches to a different Xcode installation.
