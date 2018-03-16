policy-version-range
--------------------

* The :command:`cmake_minimum_required` and :command:`cmake_policy(VERSION)`
  commands now accept a version range using the form ``<min>[...<max>]``.
  The ``<min>`` version is required but policies are set based on the
  ``<max>`` version.  This allows projects to specify a range of versions
  for which they have been updated and avoid explicit policy settings.
