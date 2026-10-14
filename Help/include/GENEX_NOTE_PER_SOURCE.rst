
Contents of |genex_subject| may use
:manual:`generator expressions <cmake-generator-expressions(7)>` with the
syntax ``$<...>``.  However, :generator:`Xcode` does not support per-config
per-source settings, so expressions that depend on the build configuration are
not allowed with that generator.
