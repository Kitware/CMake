#ifndef cmSourceFilesRequireRule_h
#define cmSourceFilesRequireRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmSourceFilesRequireRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmSourceFilesRequireRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "SOURCE_FILES_REQUIRE";}
  virtual const char* TerseDocumentation() 
    {
      return "Add a list of source files.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "SOURCE_FILES(file1 file2 ...)";
    }
};



#endif
