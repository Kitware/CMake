#ifndef cmAuxSourceDirectoryRule_h
#define cmAuxSourceDirectoryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmAuxSourceDirectoryRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmAuxSourceDirectoryRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "AUX_SOURCE_DIRECTORY";}
  virtual const char* TerseDocumentation() 
    {
      return "Add all the source files found in the specified directory to\n"
        " the build.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "AUX_SOURCE_DIRECTORY(dir)";
    }
};



#endif
