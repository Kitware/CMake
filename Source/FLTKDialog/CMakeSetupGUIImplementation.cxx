
#include "CMakeSetupGUIImplementation.h"
#include "FL/fl_file_chooser.H"
#include "FL/filename.H"
#include "FL/fl_ask.H"
#include "cstring"
#include "../cmCacheManager.h"
#include "../cmMakefile.h"
#include <iostream>
#include "FLTKPropertyList.h"
#include "FL/fl_draw.H"
#include "../cmake.h"



/**
 * Constructor
 */
CMakeSetupGUIImplementation
::CMakeSetupGUIImplementation():m_CacheEntriesList( this )
{
  m_BuildPathChanged = false;
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

  SetBinaryPath( path );

}





/**
 * Set path to executable. Used to get the path to CMake
 */
void
CMakeSetupGUIImplementation
::SetPathToExecutable( const char * path )
{
  m_PathToExecutable = path;
  
  char expandedPath[1024];
  filename_expand( expandedPath, path );
  
  char absolutePath[1024];
  filename_absolute( absolutePath, expandedPath );

  char * p = absolutePath + strlen( absolutePath );
  while( *p != '/'  && *p != '\\' ) 
  {
    p--;
  }
  p--;
  while( *p != '/'  && *p != '\\' ) 
  {
    p--;
  }
  *p = '\0';
  
  m_PathToExecutable = absolutePath;

#if defined(_WIN32)
  m_PathToExecutable += "/Debug/CMake.exe";
#else
  m_PathToExecutable += "/cmake";
#endif

}



/**
 * Set the source path
 */
bool
CMakeSetupGUIImplementation
::SetSourcePath( const char * path )
{

  if( !path || strlen(path)==0 )
  {
    fl_alert("Please select the path to the sources");
    return false; 
  }

  std::string expandedAbsolutePath = ExpandPathAndMakeItAbsolute( path );
  
  sourcePathTextInput->value( expandedAbsolutePath.c_str() );
    
  if( VerifySourcePath( expandedAbsolutePath ) )
  {
    m_WhereSource = expandedAbsolutePath;
    return true;
  }

  return false;

}




/**
 * Expand environment variables in the path and make it absolute
 */
std::string
CMakeSetupGUIImplementation
::ExpandPathAndMakeItAbsolute( const std::string & inputPath ) const
{

  char expandedPath[3000];
  filename_expand( expandedPath, inputPath.c_str() );

  char absolutePath[3000];
  filename_absolute( absolutePath, expandedPath );
  
  std::string expandedAbsolutePath = absolutePath;

  return expandedAbsolutePath;
    
}


/**
 * Set the binary path
 */
bool
CMakeSetupGUIImplementation
::SetBinaryPath( const char * path )
{

  if( !path || strlen(path)==0 )
  {
    fl_alert("Please select the path to the binaries");
    return false;
  }

  std::string expandedAbsolutePath = ExpandPathAndMakeItAbsolute( path );
  
  binaryPathTextInput->value( expandedAbsolutePath.c_str() );

  if( !VerifyBinaryPath( expandedAbsolutePath.c_str() ) )
  {
    return false;
  }

  if( m_WhereBuild != expandedAbsolutePath )
  {
    m_BuildPathChanged = true;
    m_WhereBuild = expandedAbsolutePath;
  }
  
  LoadCacheFromDiskToGUI();

  return true;

}



/**
 * Verify the path to binaries
 */
bool
CMakeSetupGUIImplementation
::VerifyBinaryPath( const std::string & path ) const
{

  bool pathIsOK = false;

  if( filename_isdir( path.c_str() ) )
  {
    pathIsOK = true;
  }
  else
  {
    int userWantsToCreateDirectory = 
      fl_ask("The directory \n %s \n Doesn't exist. Do you want to create it ?",
              path.c_str() );
    
    if( userWantsToCreateDirectory  )
    {
      std::string command = "mkdir ";
      command += path;
      system( command.c_str() );
      pathIsOK = true;
    }
    else
    {
      pathIsOK = false; 
    }
  }

  return pathIsOK;

}



/**
 * Verify the path to sources
 */
bool
CMakeSetupGUIImplementation
::VerifySourcePath( const std::string & path ) const
{

  if( !filename_isdir( path.c_str() ) )
  {
    fl_alert("The Source directory \n %s \n Doesn't exist or is not a directory", path.c_str() );
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

  // Take and verify the source path from the GUI
  if( !SetSourcePath( sourcePathTextInput->value() ) )
  { 
    return;
  }
  
  // Take and verify the binary path from the GUI
  if( !SetBinaryPath( binaryPathTextInput->value() ) )
  {
    return;
  }
  

  // set the wait cursor
  fl_cursor(FL_CURSOR_WAIT,FL_BLACK,FL_WHITE);

  // save the current GUI values to the cache
  this->SaveCacheFromGUI();

  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI();

  // create a cmake object
  cmake make;
  // create the arguments for the cmake object
  std::vector<std::string> args;
  args.push_back( m_PathToExecutable.c_str() );
  std::string arg;
  arg = "-H";
  arg += m_WhereSource;
  args.push_back(arg);
  arg = "-B";
  arg += m_WhereBuild;
  args.push_back(arg);
  // run the generate process
  if(make.Generate(args) != 0)
    {
    cmSystemTools::Error(
      "Error in generation process, project files may be invalid");
    }
  // update the GUI with any new values in the caused by the
  // generation process
  this->LoadCacheFromDiskToGUI();

  // path is up-to-date now
  m_BuildPathChanged = false;

  // put the cursor back
  fl_cursor(FL_CURSOR_DEFAULT,FL_BLACK,FL_WHITE);
  fl_message("Done !");

}




/**
 * Load Cache from disk to GUI
 */
void
CMakeSetupGUIImplementation
::LoadCacheFromDiskToGUI( void )
{
  
    
  if( m_WhereBuild != "" )
    {
    cmCacheManager::GetInstance()->LoadCache( m_WhereBuild.c_str() );
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
  this->FillCacheManagerFromCacheGUI();
  if( m_WhereBuild != "" )
  {
    cmCacheManager::GetInstance()->SaveCache( 
                                  m_WhereBuild.c_str() );
  }
}


/**
 * Fill Cache GUI from cache manager
 */
void
CMakeSetupGUIImplementation
::FillCacheGUIFromCacheManager( void )
{

  // Prepare to add rows to the scroll
  m_CacheEntriesList.RemoveAll();
  propertyListPack->clear();
  propertyListPack->begin();

  const cmCacheManager::CacheEntryMap &cache =
    cmCacheManager::GetInstance()->GetCacheMap();
  for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
      i != cache.end(); ++i)
    {
    const char* key = i->first.c_str();
    const cmCacheManager::CacheEntry& value = i->second;
    
    switch(value.m_Type )
      {
      case cmCacheManager::BOOL:
        if(cmCacheManager::GetInstance()->IsOn(key))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         value.m_HelpString.c_str(),
                                         fltk::PropertyList::CHECKBOX,"");
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         value.m_HelpString.c_str(),
                                         fltk::PropertyList::CHECKBOX,"");
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::PATH,"");
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::FILE,"");
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::EDIT,"");
        break;
      case cmCacheManager::INTERNAL:
        // These entries should not be seen by the user
        break;
      }

    }

  propertyListPack->end();
  propertyListPack->init_sizes();
  cacheValuesScroll->position( 0, 0 );

  propertyListPack->redraw();

  Fl::check();

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



/**
 * Fill cache manager from Cache GUI 
 */
void
CMakeSetupGUIImplementation
::FillCacheManagerFromCacheGUI( void )
{
  cmCacheManager::GetInstance()->GetCacheMap();
  std::set<fltk::PropertyItem*> items = m_CacheEntriesList.GetItems();
  for(std::set<fltk::PropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
      fltk::PropertyItem* item = *i; 
      cmCacheManager::CacheEntry *entry = 
        cmCacheManager::GetInstance()->GetCacheEntry(
          (const char*)item->m_propName.c_str() );
      if (entry)
        {
        entry->m_Value = item->m_curValue;
        }
    }

}


