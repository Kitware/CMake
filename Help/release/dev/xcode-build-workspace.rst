xcode-build-workspace
---------------------

* The :ref:`cmake --build <Build Tool Mode>` command-line tool, when used
  with the :generator:`Xcode` generator, now detects when a third-party
  tool has wrapped the generated ``.xcodeproj`` in a ``.xcworkspace``,
  and drives the build through the workspace instead.
