
#ifndef CMakeSetupGUIImplementation_h
#define CMakeSetupGUIImplementation_h

#include "CMakeSetupGUI.h"


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
  virtual void BuildProjectFiles( void );
  virtual void BrowseForBinaryPath( void );
  virtual void BrowseForSourcePath( void );
  virtual void SetBinaryPath( const char * path );
  virtual void SetSourcePath( const char * path );
  virtual bool VerifyBinaryPath( const char * path );
  virtual bool VerifySourcePath( const char * path );

};


#endif
