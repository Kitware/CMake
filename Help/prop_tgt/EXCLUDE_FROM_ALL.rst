EXCLUDE_FROM_ALL
----------------

Exclude the target from the all target.

A property on a target that indicates if the target is excluded from
the default build target.  If it is not, then with a Makefile for
example typing make will cause this target to be built.  The same
concept applies to the default build of other generators.

With ``EXCLUDE_FROM_ALL`` set to false or not set at all, the target
will be brought up to date as part of doing a ``make install`` or its
equivalent for the CMake generator being used.  If a target has
``EXCLUDE_FROM_ALL`` set to true, then any attempt to install that
target has undefined behavior.  Note that such a target can still safely
be listed in an :command:`install(TARGETS)` command as long as the install
components the target belongs to are not part of the set of components
that anything tries to install.

This property is enabled by default for targets that are created in
directories that have :prop_dir:`EXCLUDE_FROM_ALL` set to ``TRUE``.
