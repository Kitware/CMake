#ifndef cmMakeFileGenerator_h
#define cmMakeFileGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
struct cmClassFile;

class cmMakefileGenerator
{
public:
  // use the m_Makefile and the m_CustomRules and m_ExtraSourceFiles
  // to generate the makefile
  virtual void GenerateMakefile() = 0;
  void SetMakefile(cmMakefile*);
protected:
  cmMakefile* m_Makefile;
};

#endif
