#ifndef cmRuleMaker_h
#define cmRuleMaker_h
#include "cmStandardIncludes.h"
#include "cmMakefile.h"

class cmRuleMaker
{
public:
  cmRuleMaker()  {  m_Makefile = 0; m_Enabled = true; }
  void SetMakefile(cmMakefile*m) {m_Makefile = m; }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args) = 0;
  // This is called after the entire file has been parsed.
  virtual void FinalPass() = 0;
  // This is called to let the rule check the cache
  virtual void LoadCache() {  }
  
  virtual cmRuleMaker* Clone() = 0;
  
  // This determines if the rule gets passed down
  // to sub directory makefiles
  virtual bool IsInherited() 
    {
      return false;
    }
  // This is the name used in the input file.
  virtual const char* GetName() = 0;
  // Return the terse documentaion for the rule
  virtual const char* TerseDocumentaion() = 0;
  // Return full documentation for the rule
  virtual const char* FullDocumentaion() = 0;
  // enable or disable this rule
  bool GetEnabled()  { return m_Enabled; }
  void SetEnableOn() { m_Enabled = true; }
  void SetEnableOff() {  m_Enabled = false; }
  const char* GetError() { return m_Error.c_str();}
protected:
  void SetError(const char* e)
    {
      m_Error = this->GetName();
      m_Error += " ";
      m_Error += e;
    }
  cmMakefile* m_Makefile;
private:
  bool m_Enabled;
  std::string m_Error;
};


#endif
