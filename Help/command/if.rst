if
--

Conditionally execute a group of commands.

::

  if(expression)
    # then section.
    COMMAND1(ARGS ...)
    COMMAND2(ARGS ...)
    ...
  elseif(expression2)
    # elseif section.
    COMMAND1(ARGS ...)
    COMMAND2(ARGS ...)
    ...
  else(expression)
    # else section.
    COMMAND1(ARGS ...)
    COMMAND2(ARGS ...)
    ...
  endif(expression)

Evaluates the given expression.  If the result is true, the commands
in the THEN section are invoked.  Otherwise, the commands in the else
section are invoked.  The elseif and else sections are optional.  You
may have multiple elseif clauses.  Note that the expression in the
else and endif clause is optional.  Long expressions can be used and
there is a traditional order of precedence.  Parenthetical expressions
are evaluated first followed by unary operators such as EXISTS,
COMMAND, and DEFINED.  Then any EQUAL, LESS, GREATER, STRLESS,
STRGREATER, STREQUAL, MATCHES will be evaluated.  Then NOT operators
and finally AND, OR operators will be evaluated.  Possible expressions
are:

::

  if(<constant>)

True if the constant is 1, ON, YES, TRUE, Y, or a non-zero number.
False if the constant is 0, OFF, NO, FALSE, N, IGNORE, NOTFOUND, '',
or ends in the suffix '-NOTFOUND'.  Named boolean constants are
case-insensitive.  If the argument is not one of these constants, it
is treated as a variable:

::

  if(<variable>)

True if the variable is defined to a value that is not a false
constant.  False otherwise.  (Note macro arguments are not variables.)

::

  if(NOT <expression>)

True if the expression is not true.

::

  if(<expr1> AND <expr2>)

True if both expressions would be considered true individually.

::

  if(<expr1> OR <expr2>)

True if either expression would be considered true individually.

::

  if(COMMAND command-name)

True if the given name is a command, macro or function that can be
invoked.

::

  if(POLICY policy-id)

True if the given name is an existing policy (of the form CMP<NNNN>).

::

  if(TARGET target-name)

True if the given name is an existing target, built or imported.

::

  if(EXISTS file-name)
  if(EXISTS directory-name)

True if the named file or directory exists.  Behavior is well-defined
only for full paths.

::

  if(file1 IS_NEWER_THAN file2)

True if file1 is newer than file2 or if one of the two files doesn't
exist.  Behavior is well-defined only for full paths.  If the file
time stamps are exactly the same, an IS_NEWER_THAN comparison returns
true, so that any dependent build operations will occur in the event
of a tie.  This includes the case of passing the same file name for
both file1 and file2.

::

  if(IS_DIRECTORY directory-name)

True if the given name is a directory.  Behavior is well-defined only
for full paths.

::

  if(IS_SYMLINK file-name)

True if the given name is a symbolic link.  Behavior is well-defined
only for full paths.

::

  if(IS_ABSOLUTE path)

True if the given path is an absolute path.

::

  if(<variable|string> MATCHES regex)

True if the given string or variable's value matches the given regular
expression.

::

  if(<variable|string> LESS <variable|string>)
  if(<variable|string> GREATER <variable|string>)
  if(<variable|string> EQUAL <variable|string>)

True if the given string or variable's value is a valid number and the
inequality or equality is true.

::

  if(<variable|string> STRLESS <variable|string>)
  if(<variable|string> STRGREATER <variable|string>)
  if(<variable|string> STREQUAL <variable|string>)

True if the given string or variable's value is lexicographically less
(or greater, or equal) than the string or variable on the right.

::

  if(<variable|string> VERSION_LESS <variable|string>)
  if(<variable|string> VERSION_EQUAL <variable|string>)
  if(<variable|string> VERSION_GREATER <variable|string>)

Component-wise integer version number comparison (version format is
major[.minor[.patch[.tweak]]]).

::

  if(DEFINED <variable>)

True if the given variable is defined.  It does not matter if the
variable is true or false just if it has been set.

::

  if((expression) AND (expression OR (expression)))

The expressions inside the parenthesis are evaluated first and then
the remaining expression is evaluated as in the previous examples.
Where there are nested parenthesis the innermost are evaluated as part
of evaluating the expression that contains them.

The if command was written very early in CMake's history, predating
the ${} variable evaluation syntax, and for convenience evaluates
variables named by its arguments as shown in the above signatures.
Note that normal variable evaluation with ${} applies before the if
command even receives the arguments.  Therefore code like

::

  set(var1 OFF)
  set(var2 "var1")
  if(${var2})

appears to the if command as

::

  if(var1)

and is evaluated according to the if(<variable>) case documented
above.  The result is OFF which is false.  However, if we remove the
${} from the example then the command sees

::

  if(var2)

which is true because var2 is defined to "var1" which is not a false
constant.

Automatic evaluation applies in the other cases whenever the
above-documented signature accepts <variable|string>:

1) The left hand argument to MATCHES is first checked to see if it is
a defined variable, if so the variable's value is used, otherwise the
original value is used.

2) If the left hand argument to MATCHES is missing it returns false
without error

3) Both left and right hand arguments to LESS GREATER EQUAL are
independently tested to see if they are defined variables, if so their
defined values are used otherwise the original value is used.

4) Both left and right hand arguments to STRLESS STREQUAL STRGREATER
are independently tested to see if they are defined variables, if so
their defined values are used otherwise the original value is used.

5) Both left and right hand argumemnts to VERSION_LESS VERSION_EQUAL
VERSION_GREATER are independently tested to see if they are defined
variables, if so their defined values are used otherwise the original
value is used.

6) The right hand argument to NOT is tested to see if it is a boolean
constant, if so the value is used, otherwise it is assumed to be a
variable and it is dereferenced.

7) The left and right hand arguments to AND OR are independently
tested to see if they are boolean constants, if so they are used as
such, otherwise they are assumed to be variables and are dereferenced.
