#ifndef cmWin32LibrariesRule_h
#define cmWin32LibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmWin32LibrariesRule  : public cmRuleMaker
{
public:
  cmWin32LibrariesRule();
  virtual cmRuleMaker* Clone() 
    {
      return new cmWin32LibrariesRule ;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "WIN32_LIBRARIES";}
  virtual const char* TerseDocumentation() 
    {
      return "Set a name for the library to be built. One argument.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Set the name for the library in this makefile.  \n"
        "This takes one argument.\n"
        "LIBRARY(libraryname);\n"
        "There can be only one library per CMakeLists.txt file.\n";
    }
};



#endif
