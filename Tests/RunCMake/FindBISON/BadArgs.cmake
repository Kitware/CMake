find_package(BISON REQUIRED)
bison_target(Unparsed parser.y parser1.c UNKNOWN)
bison_target(VerboseTwoFiles parser.y parser2.c VERBOSE a.output b.output)
