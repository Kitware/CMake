#ifndef cmQtAutomoc_h
#define cmQtAutomoc_h

class cmGlobalGenerator;
class cmMakefile;

class cmQtAutomoc
{
public:
  cmQtAutomoc();
  bool Run(const char* targetDirectory);

private:
  cmGlobalGenerator* CreateGlobalGenerator(cmake* cm,
                                           const char* targetDirectory);

  bool ReadAutomocInfoFile(cmMakefile* makefile,
                           const char* targetDirectory);
  bool ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                 const char* targetDirectory);
  void WriteOldMocDefinitionsFile(const char* targetDirectory);

  bool RunAutomocQt4();

  std::string QtMajorVersion;

};

#endif
