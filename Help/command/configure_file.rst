configure_file
--------------

Copy a file to another location and modify its contents.

::

  configure_file(<input> <output>
                 [COPYONLY] [ESCAPE_QUOTES] [@ONLY]
                 [NEWLINE_STYLE [UNIX|DOS|WIN32|LF|CRLF] ])

Copies a file <input> to file <output> and substitutes variable values
referenced in the file content.  If <input> is a relative path it is
evaluated with respect to the current source directory.  The <input>
must be a file, not a directory.  If <output> is a relative path it is
evaluated with respect to the current binary directory.  If <output>
names an existing directory the input file is placed in that directory
with its original name.

If the <input> file is modified the build system will re-run CMake to
re-configure the file and generate the build system again.

This command replaces any variables in the input file referenced as
${VAR} or @VAR@ with their values as determined by CMake.  If a
variable is not defined, it will be replaced with nothing.  If
COPYONLY is specified, then no variable expansion will take place.  If
ESCAPE_QUOTES is specified then any substituted quotes will be C-style
escaped.  The file will be configured with the current values of CMake
variables.  If @ONLY is specified, only variables of the form @VAR@
will be replaced and ${VAR} will be ignored.  This is useful for
configuring scripts that use ${VAR}.

Input file lines of the form "#cmakedefine VAR ..." will be replaced
with either "#define VAR ..." or "/* #undef VAR */" depending on
whether VAR is set in CMake to any value not considered a false
constant by the if() command.  (Content of "...", if any, is processed
as above.) Input file lines of the form "#cmakedefine01 VAR" will be
replaced with either "#define VAR 1" or "#define VAR 0" similarly.

With NEWLINE_STYLE the line ending could be adjusted:

::

    'UNIX' or 'LF' for \n, 'DOS', 'WIN32' or 'CRLF' for \r\n.

COPYONLY must not be used with NEWLINE_STYLE.
