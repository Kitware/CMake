#ifndef cmIncludeDirectoryRule_h
#define cmIncludeDirectoryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmIncludeDirectoryRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmIncludeDirectoryRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "INCLUDE_DIRECTORIES";}
  virtual const char* TerseDocumentation() 
    {
      return "Add include directories to the build.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "INCLUDE_DIRECTORIES(dir1 dir2 ...).\n";
    }
};



#endif
