/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CMakeSetupGUIImplementation_h
#define CMakeSetupGUIImplementation_h

#include "CMakeSetupGUI.h"
#include "FLTKPropertyList.h"
#include <set>


/**
 *
 *  This class implements the virtual methods 
 *  declared in the GUI interface
 *
 */
   
class CMakeSetupGUIImplementation : public CMakeSetupGUI 
{

public:

  CMakeSetupGUIImplementation();
  virtual ~CMakeSetupGUIImplementation();
  virtual void Close( void );
  virtual void Show( void );
  virtual void UpdateData( bool option );
  virtual void BrowseForBinaryPath( void );
  virtual void BrowseForSourcePath( void );
  virtual bool SetBinaryPath( const char * path );
  virtual bool SetSourcePath( const char * path );
  virtual void SaveCacheFromGUI( void );
  virtual void LoadCacheFromDiskToGUI( void );
  virtual void FillCacheGUIFromCacheManager( void );
  virtual void FillCacheManagerFromCacheGUI( void );
	virtual void SetPathToExecutable( const char * path );
  virtual void LoadRecentDirectories(void);
  virtual void SaveRecentDirectories(void);
  virtual void ShowRecentBinaryDirectories(void);
  virtual void ShowRecentSourceDirectories(void);
  virtual void SelectOneRecentSourceDirectory(void);
  virtual void SelectOneRecentBinaryDirectory(void);
  virtual void UpdateListOfRecentDirectories(void);
  virtual void ClickOnConfigure(void);
  virtual void ClickOnOK(void);
  virtual void ClickOnCancel(void);
  virtual void RunCMake( bool generateProjectFiles );

private:  
  virtual bool VerifyBinaryPath( const std::string & path ) const;
  virtual bool VerifySourcePath( const std::string & path ) const;
  virtual std::string ExpandPathAndMakeItAbsolute( const std::string & inputPath ) const;

private:
  fltk::PropertyList   m_CacheEntriesList;
  std::string          m_WhereBuild;
  std::string          m_WhereSource;
  std::string          m_PathToExecutable;
  std::string	         m_GeneratorChoiceString;
  bool                 m_BuildPathChanged;
  
  std::set< std::string > m_RecentBinaryDirectories;
  std::set< std::string > m_RecentSourceDirectories;

};


#endif
