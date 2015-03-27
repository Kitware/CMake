add_dependencies-INTERFACE-libraries
------------------------------------

* The :command:`add_dependencies` command learned to allow dependencies
  to be added to :ref:`interface libraries <Interface Libraries>`.
  Dependencies added to an interface library are followed transitively
  in its place since the target itself does not build.
