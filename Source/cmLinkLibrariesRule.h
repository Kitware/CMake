#ifndef cmLinkLibrariesRule_h
#define cmLinkLibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmLinkLibrariesRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmLinkLibrariesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }
  
    
  // This is the name used in the input file.
  virtual const char* GetName() { return "LINK_LIBRARIES";}
  virtual const char* TerseDocumentation() 
    {
      return 
        "Specify a list of libraries to be linked into executables or \n"
        "shared objects.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Specify a list of libraries to be linked into executables or \n"
        "shared objects.  This rule is passed down to all other rules."
        "LINK_LIBRARIES(library1 library2).\n"
        "The library name should be the same as the name used in the\n"
        "LIBRARY(library) rule.";
    }
};



#endif
