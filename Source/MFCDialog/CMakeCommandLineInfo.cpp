// CMakeCommandLineInfo.cpp : command line arguments
//

#include "stdafx.h"
#include "CMakeCommandLineInfo.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

///////////////////////////////////////////////////////////////
// CMakeCommandLineInfo 

CMakeCommandLineInfo::CMakeCommandLineInfo()
{
  m_WhereSource = _T("");
  m_WhereBuild = _T("");
  m_AdvancedValues = FALSE;
  m_GeneratorChoiceString = _T("");
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
  if(bFlag) 
    {
    CString sParam(lpszParam);
    // Single letter valued flag like /B=value or /B:value
    if (sParam[1] == '=' || sParam[1] == ':')
      {
      CString value(sParam.Right(sParam.GetLength() - 2));
      int res;
      switch (sParam[0])
        {
        case 'A':
          res = CMakeCommandLineInfo::GetBoolValue(value);
          if (res == 1)
            {
            m_AdvancedValues = TRUE;
            }
          else if (res == -1)
            {
            m_AdvancedValues = FALSE;
            }
          break;
        case 'B':
          m_WhereBuild = value;
          break;
        case 'G':
          m_GeneratorChoiceString = value;
          break;
        case 'H':
          m_WhereSource = value;
          break;
        }
      }
    }

  // Call the base class to ensure proper command line processing
  CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}
