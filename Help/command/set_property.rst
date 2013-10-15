set_property
------------

Set a named property in a given scope.

::

  set_property(<GLOBAL                            |
                DIRECTORY [dir]                   |
                TARGET    [target1 [target2 ...]] |
                SOURCE    [src1 [src2 ...]]       |
                TEST      [test1 [test2 ...]]     |
                CACHE     [entry1 [entry2 ...]]>
               [APPEND] [APPEND_STRING]
               PROPERTY <name> [value1 [value2 ...]])

Set one property on zero or more objects of a scope.  The first
argument determines the scope in which the property is set.  It must
be one of the following:

GLOBAL scope is unique and does not accept a name.

DIRECTORY scope defaults to the current directory but another
directory (already processed by CMake) may be named by full or
relative path.

TARGET scope may name zero or more existing targets.

SOURCE scope may name zero or more source files.  Note that source
file properties are visible only to targets added in the same
directory (CMakeLists.txt).

TEST scope may name zero or more existing tests.

CACHE scope must name zero or more cache existing entries.

The required PROPERTY option is immediately followed by the name of
the property to set.  Remaining arguments are used to compose the
property value in the form of a semicolon-separated list.  If the
APPEND option is given the list is appended to any existing property
value.If the APPEND_STRING option is given the string is append to any
existing property value as string, i.e.  it results in a longer string
and not a list of strings.
