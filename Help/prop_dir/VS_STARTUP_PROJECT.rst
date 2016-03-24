VS_STARTUP_PROJECT
------------------

Specify the default startup project in a Visual Studio solution.

The property must be set to the name of an existing target.  This
will cause that project to be listed first in the generated solution
file causing Visual Studio to make it the startup project if the
solution has never been opened before.

If this property is not specified, then the "ALL_BUILD" project
will be the default.
