#ifndef cmSubdirRule_h
#define cmSubdirRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmSubdirRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmSubdirRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "SUBDIRS";}
  virtual const char* TerseDocumentation() 
    {
      return "Add a list of subdirectories to the build.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Add a list of subdirectories to the build.\n"
        "SUBDIRS(dir1 dir2 ...)\n"
        "This will cause any CMakeLists.txt files in the sub directories\n"
        "to be parsed by cmake.";
    }
};



#endif
