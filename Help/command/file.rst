file
----

File manipulation command.

::

  file(WRITE filename "message to write"... )
  file(APPEND filename "message to write"... )
  file(READ filename variable [LIMIT numBytes] [OFFSET offset] [HEX])
  file(<MD5|SHA1|SHA224|SHA256|SHA384|SHA512> filename variable)
  file(STRINGS filename variable [LIMIT_COUNT num]
       [LIMIT_INPUT numBytes] [LIMIT_OUTPUT numBytes]
       [LENGTH_MINIMUM numBytes] [LENGTH_MAXIMUM numBytes]
       [NEWLINE_CONSUME] [REGEX regex]
       [NO_HEX_CONVERSION])
  file(GLOB variable [RELATIVE path] [globbing expressions]...)
  file(GLOB_RECURSE variable [RELATIVE path]
       [FOLLOW_SYMLINKS] [globbing expressions]...)
  file(RENAME <oldname> <newname>)
  file(REMOVE [file1 ...])
  file(REMOVE_RECURSE [file1 ...])
  file(MAKE_DIRECTORY [directory1 directory2 ...])
  file(RELATIVE_PATH variable directory file)
  file(TO_CMAKE_PATH path result)
  file(TO_NATIVE_PATH path result)
  file(DOWNLOAD url file [INACTIVITY_TIMEOUT timeout]
       [TIMEOUT timeout] [STATUS status] [LOG log] [SHOW_PROGRESS]
       [EXPECTED_HASH ALGO=value] [EXPECTED_MD5 sum]
       [TLS_VERIFY on|off] [TLS_CAINFO file])
  file(UPLOAD filename url [INACTIVITY_TIMEOUT timeout]
       [TIMEOUT timeout] [STATUS status] [LOG log] [SHOW_PROGRESS])
  file(TIMESTAMP filename variable [<format string>] [UTC])
  file(GENERATE OUTPUT output_file
       <INPUT input_file|CONTENT input_content>
       [CONDITION expression])

WRITE will write a message into a file called 'filename'.  It
overwrites the file if it already exists, and creates the file if it
does not exist.  (If the file is a build input, use configure_file to
update the file only when its content changes.)

APPEND will write a message into a file same as WRITE, except it will
append it to the end of the file

READ will read the content of a file and store it into the variable.
It will start at the given offset and read up to numBytes.  If the
argument HEX is given, the binary data will be converted to
hexadecimal representation and this will be stored in the variable.

MD5, SHA1, SHA224, SHA256, SHA384, and SHA512 will compute a
cryptographic hash of the content of a file.

STRINGS will parse a list of ASCII strings from a file and store it in
a variable.  Binary data in the file are ignored.  Carriage return
(CR) characters are ignored.  It works also for Intel Hex and Motorola
S-record files, which are automatically converted to binary format
when reading them.  Disable this using NO_HEX_CONVERSION.

LIMIT_COUNT sets the maximum number of strings to return.  LIMIT_INPUT
sets the maximum number of bytes to read from the input file.
LIMIT_OUTPUT sets the maximum number of bytes to store in the output
variable.  LENGTH_MINIMUM sets the minimum length of a string to
return.  Shorter strings are ignored.  LENGTH_MAXIMUM sets the maximum
length of a string to return.  Longer strings are split into strings
no longer than the maximum length.  NEWLINE_CONSUME allows newlines to
be included in strings instead of terminating them.

REGEX specifies a regular expression that a string must match to be
returned.  Typical usage

::

  file(STRINGS myfile.txt myfile)

stores a list in the variable "myfile" in which each item is a line
from the input file.

GLOB will generate a list of all files that match the globbing
expressions and store it into the variable.  Globbing expressions are
similar to regular expressions, but much simpler.  If RELATIVE flag is
specified for an expression, the results will be returned as a
relative path to the given path.  (We do not recommend using GLOB to
collect a list of source files from your source tree.  If no
CMakeLists.txt file changes when a source is added or removed then the
generated build system cannot know when to ask CMake to regenerate.)

Examples of globbing expressions include:

::

   *.cxx      - match all files with extension cxx
   *.vt?      - match all files with extension vta,...,vtz
   f[3-5].txt - match files f3.txt, f4.txt, f5.txt

GLOB_RECURSE will generate a list similar to the regular GLOB, except
it will traverse all the subdirectories of the matched directory and
match the files.  Subdirectories that are symlinks are only traversed
if FOLLOW_SYMLINKS is given or cmake policy CMP0009 is not set to NEW.
See cmake --help-policy CMP0009 for more information.

Examples of recursive globbing include:

::

   /dir/*.py  - match all python files in /dir and subdirectories

MAKE_DIRECTORY will create the given directories, also if their parent
directories don't exist yet

RENAME moves a file or directory within a filesystem, replacing the
destination atomically.

REMOVE will remove the given files, also in subdirectories

REMOVE_RECURSE will remove the given files and directories, also
non-empty directories

RELATIVE_PATH will determine relative path from directory to the given
file.

TO_CMAKE_PATH will convert path into a cmake style path with unix /.
The input can be a single path or a system path like "$ENV{PATH}".
Note the double quotes around the ENV call TO_CMAKE_PATH only takes
one argument.  This command will also convert the native list
delimiters for a list of paths like the PATH environment variable.

TO_NATIVE_PATH works just like TO_CMAKE_PATH, but will convert from a
cmake style path into the native path style \ for windows and / for
UNIX.

DOWNLOAD will download the given URL to the given file.  If LOG var is
specified a log of the download will be put in var.  If STATUS var is
specified the status of the operation will be put in var.  The status
is returned in a list of length 2.  The first element is the numeric
return value for the operation, and the second element is a string
value for the error.  A 0 numeric error means no error in the
operation.  If TIMEOUT time is specified, the operation will timeout
after time seconds, time should be specified as an integer.  The
INACTIVITY_TIMEOUT specifies an integer number of seconds of
inactivity after which the operation should terminate.  If
EXPECTED_HASH ALGO=value is specified, the operation will verify that
the downloaded file's actual hash matches the expected value, where
ALGO is one of MD5, SHA1, SHA224, SHA256, SHA384, or SHA512.  If it
does not match, the operation fails with an error.  ("EXPECTED_MD5
sum" is short-hand for "EXPECTED_HASH MD5=sum".) If SHOW_PROGRESS is
specified, progress information will be printed as status messages
until the operation is complete.  For https URLs CMake must be built
with OpenSSL.  TLS/SSL certificates are not checked by default.  Set
TLS_VERIFY to ON to check certificates and/or use EXPECTED_HASH to
verify downloaded content.  Set TLS_CAINFO to specify a custom
Certificate Authority file.  If either TLS option is not given CMake
will check variables CMAKE_TLS_VERIFY and CMAKE_TLS_CAINFO,
respectively.

UPLOAD will upload the given file to the given URL.  If LOG var is
specified a log of the upload will be put in var.  If STATUS var is
specified the status of the operation will be put in var.  The status
is returned in a list of length 2.  The first element is the numeric
return value for the operation, and the second element is a string
value for the error.  A 0 numeric error means no error in the
operation.  If TIMEOUT time is specified, the operation will timeout
after time seconds, time should be specified as an integer.  The
INACTIVITY_TIMEOUT specifies an integer number of seconds of
inactivity after which the operation should terminate.  If
SHOW_PROGRESS is specified, progress information will be printed as
status messages until the operation is complete.

TIMESTAMP will write a string representation of the modification time
of filename to variable.

Should the command be unable to obtain a timestamp variable will be
set to the empty string "".

See documentation of the string TIMESTAMP sub-command for more
details.

The file() command also provides COPY and INSTALL signatures:

::

  file(<COPY|INSTALL> files... DESTINATION <dir>
       [FILE_PERMISSIONS permissions...]
       [DIRECTORY_PERMISSIONS permissions...]
       [NO_SOURCE_PERMISSIONS] [USE_SOURCE_PERMISSIONS]
       [FILES_MATCHING]
       [[PATTERN <pattern> | REGEX <regex>]
        [EXCLUDE] [PERMISSIONS permissions...]] [...])

The COPY signature copies files, directories, and symlinks to a
destination folder.  Relative input paths are evaluated with respect
to the current source directory, and a relative destination is
evaluated with respect to the current build directory.  Copying
preserves input file timestamps, and optimizes out a file if it exists
at the destination with the same timestamp.  Copying preserves input
permissions unless explicit permissions or NO_SOURCE_PERMISSIONS are
given (default is USE_SOURCE_PERMISSIONS).  See the install(DIRECTORY)
command for documentation of permissions, PATTERN, REGEX, and EXCLUDE
options.

The INSTALL signature differs slightly from COPY: it prints status
messages, and NO_SOURCE_PERMISSIONS is default.  Installation scripts
generated by the install() command use this signature (with some
undocumented options for internal use).

GENERATE will write an <output_file> with content from an
<input_file>, or from <input_content>.  The output is generated
conditionally based on the content of the <condition>.  The file is
written at CMake generate-time and the input may contain generator
expressions.  The <condition>, <output_file> and <input_file> may also
contain generator expressions.  The <condition> must evaluate to
either '0' or '1'.  The <output_file> must evaluate to a unique name
among all configurations and among all invocations of file(GENERATE).
