#ifndef cmProjectRule_h
#define cmProjectRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmProjectRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmProjectRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "PROJECT";}
  virtual const char* TerseDocumentation() 
    {
      return "Set a name for the entire project. One argument.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Set the name for the entire project.  This takes one argument.\n"
        "PROJECT(projectname);";
    }
};



#endif
