WORKING_DIRECTORY
-----------------

.. versionadded:: 4.5

Execute the commands produced by the instantiation of the rule with the given
current working directory. If it is a relative path, it will be interpreted
relative to the build tree directory corresponding to the current source
directory of the target. If not specified, the default value is the build
directory corresponding to the current source directory of the target.

Arguments to ``WORKING_DIRECTORY`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.
