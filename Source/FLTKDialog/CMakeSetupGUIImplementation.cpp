
#include "CMakeSetupGUIImplementation.h"
#include "Fl/fl_file_chooser.H"
#include "Fl/filename.H"
#include "Fl/fl_ask.H"
#include "cstring"



/**
 * Constructor
 */
CMakeSetupGUIImplementation
::CMakeSetupGUIImplementation()
{
}



/**
 * Destructor
 */
CMakeSetupGUIImplementation
::~CMakeSetupGUIImplementation()
{
}



/**
 * Show the graphic interface
 */
void
CMakeSetupGUIImplementation
::Show( void )
{
  dialogWindow->show();
}





/**
 * Hide the graphic interface
 */
void
CMakeSetupGUIImplementation
::Close( void )
{
  dialogWindow->hide();
}





/**
 * Browse for the path to the sources
 */
void
CMakeSetupGUIImplementation
::BrowseForSourcePath( void )
{
  const char * path = 
                  fl_file_chooser(
                    "Path to Sources",
                    "",
                    sourcePathTextInput->value() );
                    
  if( !path )
  {
    return;
  }
  
  SetSourcePath( path );

}




/**
 * Browse for the path to the binaries
 */
void
CMakeSetupGUIImplementation
::BrowseForBinaryPath( void )
{
  const char * path = 
                  fl_file_chooser(
                    "Path to Binaries",
                    "",
                    binaryPathTextInput->value() );
                    
  if( !path )
  {
    return;
  }

  binaryPathTextInput->value( path );
}





/**
 * Set the source path
 */
void
CMakeSetupGUIImplementation
::SetSourcePath( const char * path )
{
  if( VerifySourcePath( path ) )
  {
    sourcePathTextInput->value( path );
  }

}




/**
 * Set the binary path
 */
void
CMakeSetupGUIImplementation
::SetBinaryPath( const char * path )
{

  if( VerifyBinaryPath( path ) )
  {
    binaryPathTextInput->value( path );
  }

}



/**
 * Verify the path to binaries
 */
bool
CMakeSetupGUIImplementation
::VerifyBinaryPath( const char * path )
{

  if( !path || strlen(path)==0 )
  {
    fl_alert("Please select the path to the binaries");
    return false; 
  }


  if( !filename_isdir( path ) )
  {
    fl_alert("%s \n Doesn't exist or is not a directory",path);
    return false; 
  }

  return true;
}



/**
 * Verify the path to sources
 */
bool
CMakeSetupGUIImplementation
::VerifySourcePath( const char * path )
{

  if( !path || strlen(path)==0 )
  {
    fl_alert("Please select the path to the sources");
    return false; 
  }


  if( !filename_isdir( path ) )
  {
    fl_alert("%s \n Doesn't exist or is not a directory",path);
    return false; 
  }

  return true;
}




/**
 * Build the project files
 */
void
CMakeSetupGUIImplementation
::BuildProjectFiles( void )
{

  // Verify that source path is a valid directory
  if( !VerifySourcePath( sourcePathTextInput->value() ) )
  { 
    return;
  }

  // Verify that binary path is a valid directory
  if( !VerifyBinaryPath( binaryPathTextInput->value() ) )
  { 
    return;
  }

 fl_message("Building project files ... please wait");

}


