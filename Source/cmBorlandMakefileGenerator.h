#ifndef cmBorlandMakefileGenerator_h
#define cmBorlandMakefileGenerator_h

#include "cmMakefile.h"
#include "cmMakefileGenerator.h"

/** \class cmBorlandMakefileGenerator 
 * \brief Write Borland BCB5 compatible makefiles.
 *
 * cmBorlandMakefileGenerator produces Borland BCB5 compatible makefiles
 */
class cmBorlandMakefileGenerator : public cmMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmBorlandMakefileGenerator();
    
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Borland Makefiles";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmBorlandMakefileGenerator;}

  //! just sets the Cache Only and Recurse flags
  virtual void SetLocal(bool local);

  /**
   * If cache only is on.
   * Only stub makefiles are generated, and no depends, for speed.
   * The default is OFF.
   **/
  void SetCacheOnlyOn()  {m_CacheOnly = true;}
  void SetCacheOnlyOff() {m_CacheOnly = false;}

  /**
   * If recurse is on, then all the makefiles below this one are parsed as well.
   */
  void SetRecurseOn()  {m_Recurse = true;}
  void SetRecurseOff() {m_Recurse = false;}

  /**
   * Produce the makefile (in this case a Unix makefile).
   */
  virtual void GenerateMakefile();

  /**
   * Output the depend information for all the classes
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  void OutputObjectDepends(std::ostream&);

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void ComputeSystemInfo();

private:
  void RecursiveGenerateCacheOnly();
  void GenerateCacheOnly();
  void OutputMakefile(const char* file);
  void OutputTargetRules(std::ostream& fout);
  void OutputTargets(std::ostream&);
  void OutputSubDirectoryRules(std::ostream&);
  void OutputDependInformation(std::ostream&);
  void OutputDependencies(std::ostream&);
  void OutputCustomRules(std::ostream&);
  void OutputMakeVariables(std::ostream&);
  void OutputMakeRules(std::ostream&);
  void OutputSubDirectoryVars(std::ostream& fout,
                              const char* var,
                              const char* target,
                              const char* target1,
                              const char* target2,
                              const std::vector<std::string>& SubDirectories);
  void OutputMakeRule(std::ostream&,
                      const char* comment,
                      const char* target,
                      const char* depends,
                      const char* command);
private:
  bool m_CacheOnly;
  bool m_Recurse;
  std::string m_ExecutableOutputPath;
  std::string m_LibraryOutputPath;
};

#endif
