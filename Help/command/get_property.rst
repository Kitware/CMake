get_property
------------

Get a property.

::

  get_property(<variable>
               <GLOBAL             |
                DIRECTORY [dir]    |
                TARGET    <target> |
                SOURCE    <source> |
                TEST      <test>   |
                CACHE     <entry>  |
                VARIABLE>
               PROPERTY <name>
               [SET | DEFINED | BRIEF_DOCS | FULL_DOCS])

Get one property from one object in a scope.  The first argument
specifies the variable in which to store the result.  The second
argument determines the scope from which to get the property.  It must
be one of the following:

GLOBAL scope is unique and does not accept a name.

DIRECTORY scope defaults to the current directory but another
directory (already processed by CMake) may be named by full or
relative path.

TARGET scope must name one existing target.

SOURCE scope must name one source file.

TEST scope must name one existing test.

CACHE scope must name one cache entry.

VARIABLE scope is unique and does not accept a name.

The required PROPERTY option is immediately followed by the name of
the property to get.  If the property is not set an empty value is
returned.  If the SET option is given the variable is set to a boolean
value indicating whether the property has been set.  If the DEFINED
option is given the variable is set to a boolean value indicating
whether the property has been defined such as with define_property.
If BRIEF_DOCS or FULL_DOCS is given then the variable is set to a
string containing documentation for the requested property.  If
documentation is requested for a property that has not been defined
NOTFOUND is returned.
