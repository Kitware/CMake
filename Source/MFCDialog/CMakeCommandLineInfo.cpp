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
} 

CMakeCommandLineInfo::~CMakeCommandLineInfo()
{
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
      switch (sParam[0])
        {
        case 'H':
          m_WhereSource = value;
          break;
        case 'B':
          m_WhereBuild = value;
          break;
        }
      }
    }

  // Call the base class to ensure proper command line processing
  CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}
