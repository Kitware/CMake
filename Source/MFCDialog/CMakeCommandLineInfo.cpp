// CMakeCommandLineInfo.cpp : command line arguments
//

#include "stdafx.h"
#include "CMakeCommandLineInfo.h" 
#include "cmSystemTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

///////////////////////////////////////////////////////////////
// CMakeCommandLineInfo 

CMakeCommandLineInfo::CMakeCommandLineInfo()
{
  this->m_WhereSource = _T("");
  this->m_WhereBuild = _T("");
  this->m_AdvancedValues = FALSE;
  this->m_GeneratorChoiceString = _T("");
  this->m_LastUnknownParameter = _T("");
  
  // Find the path to the CMakeSetup executable.
  char fname[4096];
  ::GetModuleFileName(0, fname, 4096);
  m_Argv0 = fname;
  m_Argv.push_back(m_Argv0.c_str());
} 

CMakeCommandLineInfo::~CMakeCommandLineInfo()
{
} 

int CMakeCommandLineInfo::GetBoolValue(const CString& v) {
  CString value = v;
  value.MakeLower();
  if (value == "1" || 
      value == "on" || 
      value == "true" || 
      value == "yes")
    {
    return 1;
    }
  else if (value == "0" || 
           value == "off" || 
           value == "false" || 
           value == "no")
    {
    return -1;
    }
  return 0;
}

///////////////////////////////////////////////////////////////
// Parse param

void CMakeCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
  // Construct the full name of the argument.
  cmStdString param = lpszParam;
  cmStdString value;
  if(bFlag)
    {
    // Since bFlag is set, either a - or a / was removed from the
    // parameter value.  Assume it was a - unless the second character
    // was a / which indicates a network path argument.
    if(param.length() > 0 && param[0] == '/')
      {
      value = "/";
      }
    else
      {
      value = "-";
      }
    }
  value += param;
  
  // Add the argument and reset the argv table in case strings were
  // moved.
  m_Arguments.push_back(value);
  m_Argv.clear();
  m_Argv.push_back(m_Argv0.c_str());
  for(unsigned int i=0; i < m_Arguments.size(); ++i)
    {
    m_Argv.push_back(m_Arguments[i].c_str());
    }
  
  // Look for known flags.
  if(!bFlag)
    {
    this->m_LastUnknownParameter = lpszParam;
    }
  else
    {
    CString sParam(lpszParam);
    // Single letter valued flag like /B=value or /B:value
    CString value;
    if (sParam[1] == '=' || sParam[1] == ':')
      {
      value = sParam.Right(sParam.GetLength() - 2);
      }
    else
      {
      value = sParam.Right(sParam.GetLength()-1);
      }
    int res;
    switch (sParam[0])
      {
      case 'A':
        res = CMakeCommandLineInfo::GetBoolValue(value);
        if (res == 1)
          {
          this->m_AdvancedValues = TRUE;
          }
        else if (res == -1)
          {
          this->m_AdvancedValues = FALSE;
          }
        break;
      case 'B':
      {
        std::string path = cmSystemTools::CollapseFullPath((const char*)value);
        this->m_WhereBuild = path.c_str();
        break;
      }
      case 'G':
        this->m_GeneratorChoiceString = value;
        break;
      case 'H':
      {
        std::string path = cmSystemTools::CollapseFullPath((const char*)value);
        this->m_WhereSource = path.c_str();
        break;
      }
      }
    }

  // Call the base class to ensure proper command line processing
  CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}
