#source_group(Base             FILES main.c)
#source_group(Base\\Sub1       FILES sub1/foo.c)
#source_group(Base\\Sub1\\Sub2 FILES sub1/foobar.c)
#source_group("" FILES foo.c)
#source_group("\\" FILES bar.c)


add_executable(SourceGroups main.c bar.c foo.c sub1/foo.c sub1/foobar.c)

