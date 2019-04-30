#ifndef DEF_SameNameImportedSubDirA
#  error "DEF_SameNameImportedSubDirA is not defined but should be!"
#endif
#ifndef DEF_SameNameImportedSubDirB
#  error "DEF_SameNameImportedSubDirB is not defined but should be!"
#endif
#ifdef DEF_TopDirImported
#  error "DEF_TopDirImported is defined but should not be!"
#endif
#ifdef DEF_SubDirC1
#  error "DEF_SubDirC1 defined but should not be"
#endif
#ifdef DEF_SubDirC2
#  error "DEF_SubDirC2 defined but should not be"
#endif

int main(void)
{
  return 0;
}
