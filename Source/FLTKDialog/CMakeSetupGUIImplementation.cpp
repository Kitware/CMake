
#include "CMakeSetupGUIImplementation.h"
#include "Fl/fl_file_chooser.H"
#include "Fl/filename.H"
#include "Fl/fl_ask.H"
#include "cstring"
#include "../cmCacheManager.h"
#include "../cmMakefile.h"



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

  SaveCacheFromGUI();
  
  fl_message("Building project files ... please wait");

}




/**
 * Load Cache from disk to GUI
 */
void
CMakeSetupGUIImplementation
::LoadCacheFromDiskToGUI( void )
{
  
  const char * m_WhereBuild = binaryPathTextInput->value();
    
  if( m_WhereBuild != "" )
    {
    cmCacheManager::GetInstance()->LoadCache(m_WhereBuild);
    
    // Make sure the internal "CMAKE" cache entry is set.
    const char* cacheValue = cmCacheManager::GetInstance()->GetCacheValue("CMAKE");
    if(!cacheValue)
      {
        // Find our own exectuable.
        std::string cMakeCMD = "\""+cmSystemTools::GetProgramPath(_pgmptr);
        cMakeCMD += "/CMakeSetupCMD.exe\"";

        // Save the value in the cache
        cmCacheManager::GetInstance()->AddCacheEntry("CMAKE",
						     cMakeCMD.c_str(),
						     "Path to CMake executable.",
						     cmCacheManager::INTERNAL);
      }
    
    this->FillCacheGUIFromCacheManager();
    }
}



/**
 * Save Cache from disk to GUI
 */
void
CMakeSetupGUIImplementation
::SaveCacheFromGUI( void )
{
}


/**
 * Fill Cache GUI from cache manager
 */
void
CMakeSetupGUIImplementation
::FillCacheGUIFromCacheManager( void )
{
  const cmCacheManager::CacheEntryMap &cache =
    cmCacheManager::GetInstance()->GetCacheMap();
  for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
      i != cache.end(); ++i)
    {
    const char* key = i->first.c_str();
    const cmCacheManager::CacheEntry& value = i->second;
    /*
    switch(value.m_Type )
      {
      case cmCacheManager::BOOL:
        if(cmCacheManager::GetInstance()->IsOn(key))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         value.m_HelpString.c_str(),
                                         CPropertyList::CHECKBOX,"");
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         value.m_HelpString.c_str(),
                                         CPropertyList::CHECKBOX,"");
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::PATH,"");
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::FILE,"");
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::EDIT,"");
        break;
      case cmCacheManager::INTERNAL:
        break;
      }
    */
    }
  this->UpdateData(false);

}


/**
 * UpdateData
 */
void
CMakeSetupGUIImplementation
::UpdateData( bool option )
{
  dialogWindow->redraw();
  Fl::check();
}



