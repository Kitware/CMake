#ifndef cmUnixLibrariesRule_h
#define cmUnixLibrariesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmUnixLibrariesRule : public cmRuleMaker
{
public:
  cmUnixLibrariesRule();
  virtual cmRuleMaker* Clone() 
    {
      return new cmUnixLibrariesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "UNIX_LIBRARIES";}
  virtual const char* TerseDocumentation() 
    {
      return "Add libraries that are only used for unix programs.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "UNIX_LIBRARIES(library -lm ...);";
    }
};



#endif
