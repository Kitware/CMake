
#ifndef CMakeSetupGUIImplementation_h
#define CMakeSetupGUIImplementation_h

#include "CMakeSetupGUI.h"
#include "FLTKPropertyList.h"


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
  virtual void BuildProjectFiles( void );
  virtual void BrowseForBinaryPath( void );
  virtual void BrowseForSourcePath( void );
  virtual void SetBinaryPath( const char * path );
  virtual void SetSourcePath( const char * path );
  virtual bool VerifyBinaryPath( const char * path );
  virtual bool VerifySourcePath( const char * path );
  virtual void SaveCacheFromGUI( void );
  virtual void LoadCacheFromDiskToGUI( void );
  virtual void FillCacheGUIFromCacheManager( void );
  virtual void FillCacheManagerFromCacheGUI( void );

private:
  fltk::PropertyList   m_CacheEntriesList;
  std::string          m_WhereBuild;
  std::string          m_WhereSource;
  std::string          m_PathToExecutable;
  bool                 m_BuildPathChanged;
};


#endif
