#ifndef cmAbstractFilesRule_h
#define cmAbstractFilesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmAbstractFilesRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmAbstractFilesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "ABSTRACT_FILES";}
  virtual const char* TerseDocumentaion() 
    {
      return "A list of abstract classes, useful for wrappers.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "ABSTRACT_FILES(file1 file2 ..)";
    }
};



#endif
