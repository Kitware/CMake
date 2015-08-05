java-updates
------------

* The :module:`FindJava` module learned to optionally find
  the ``idlj`` and ``jarsigner`` tools.

* The :module:`UseJava` module ``add_jar`` function learned
  to support response files (e.g. ``@srcs.txt``) for source
  specification.

* The :module:`UseJava` module ``install_jar`` function learned
  new ``DESTINATION`` and ``COMPONENT`` options to specify
  the corresponding :command:`install` command options.

* The :module:`UseJava` module gained a new ``create_javah``
  function to create C headers from Java classes.
