EXCLUDE_FROM_ALL
----------------

Exclude the directory from the all target of its parent.

A property on a directory that indicates if its targets are excluded
from the default build target.  If it is not, then with a Makefile for
example typing make will cause the targets to be built.  The same
concept applies to the default build of other generators.

Targets inherit the :prop_tgt:`EXCLUDE_FROM_ALL` property from the directory
that they are created in. When a directory is excluded, all of its targets will
have :prop_tgt:`EXCLUDE_FROM_ALL` set to ``TRUE``. After creating such a target
you can change its :prop_tgt:`EXCLUDE_FROM_ALL` property to ``FALSE``. This
will cause the target to be included in the default build target.
