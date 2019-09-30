EXCLUDE_FROM_ALL
----------------

Set this target property to a true (or false) value to exclude (or include)
the target from the "all" target of the containing directory and its
ancestors.  If excluded, running e.g. ``make`` in the containing directory
or its ancestors will not build the target by default.

If this target property is not set then the target will be included in
the "all" target of the containing directory.  Furthermore, it will be
included in the "all" target of its ancestor directories unless the
:prop_dir:`EXCLUDE_FROM_ALL` directory property is set.

With ``EXCLUDE_FROM_ALL`` set to false or not set at all, the target
will be brought up to date as part of doing a ``make install`` or its
equivalent for the CMake generator being used.  If a target has
``EXCLUDE_FROM_ALL`` set to true, then any attempt to install that
target has undefined behavior.  Note that such a target can still safely
be listed in an :command:`install(TARGETS)` command as long as the install
components the target belongs to are not part of the set of components
that anything tries to install.
