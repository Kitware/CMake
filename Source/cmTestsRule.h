#ifndef cmTestsRule_h
#define cmTestsRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmTestsRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmTestsRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }

  // This is the name used in the input file.
  virtual const char* GetName() { return "TESTS";}
  virtual const char* TerseDocumentation() 
    {
      return "Add a list of executables files that are run as tests.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "TESTS(file1 file2 ...)";
    }
};



#endif
