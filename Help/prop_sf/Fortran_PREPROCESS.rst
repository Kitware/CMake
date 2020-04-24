Fortran_PREPROCESS
------------------

Control whether the Fortran source file should be unconditionally preprocessed.

If unset or empty, rely on the compiler to determine whether the file
should be preprocessed. If explicitly set to ``OFF`` then the file
does not need to be preprocessed. If explicitly set to ``ON``, then
the file does need to be preprocessed as part of the compilation step.

Consider using the target-wide :prop_tgt:`Fortran_PREPROCESS` property
if all source files in a target need to be preprocessed.
