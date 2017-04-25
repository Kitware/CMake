ninja-loosen-object-deps
------------------------

* The :generator:`Ninja` generator has loosened dependencies on object
  compilation to depend on the custom targets and commands of dependent
  libraries instead of the libraries themselves. This helps projects with deep
  dependency graphs to be blocked only on their link steps at the deeper
  levels rather than also blocking object compilation on dependent link steps.
