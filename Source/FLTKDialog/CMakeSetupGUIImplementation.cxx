
#include "CMakeSetupGUIImplementation.h"
#include "FL/fl_file_chooser.H"
#include "FL/filename.H"
#include "FL/fl_ask.H"
#include "../cmCacheManager.h"
#include "../cmMakefile.h"
#include <iostream>
#include "FLTKPropertyList.h"
#include "FLTKPropertyItemRow.h"
#include "FL/fl_draw.H"
#include "../cmake.h"
#include "../cmMakefileGenerator.h"



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
  SaveRecentDirectories();
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
    m_BuildPathChanged  = true;
    m_WhereBuild        = expandedAbsolutePath;
    LoadCacheFromDiskToGUI();
  }
  else 
  {
    m_BuildPathChanged = false;
  }
  
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
::RunCMake( bool generateProjectFiles )
{

  if(!cmSystemTools::FileExists( m_WhereBuild.c_str() ))
    {
    std::string message =
      "Build directory does not exist, should I create it?\n\n"
      "Directory: ";
    message += m_WhereBuild;
    int userWantToCreateDirectory =
      fl_ask(message.c_str());
    if( userWantToCreateDirectory )
      {
      cmSystemTools::MakeDirectory( m_WhereBuild.c_str() );
      }
    else
      {
      fl_alert("Build Project aborted, nothing done.");
      return;
      }
    }

  
  // set the wait cursor
  fl_cursor(FL_CURSOR_WAIT,FL_BLACK,FL_WHITE);


  // save the current GUI values to the cache
  this->SaveCacheFromGUI();

  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI();

  UpdateListOfRecentDirectories();
  SaveRecentDirectories();

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
  arg = "-G";
  arg += m_GeneratorChoiceString;
  args.push_back(arg);
  // run the generate process
  if(make.Generate(args, generateProjectFiles) != 0)
    {
    cmSystemTools::Error(
      "Error in generation process, project files may be invalid");
    cmSystemTools::ResetErrorOccuredFlag();
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
  int size = m_CacheEntriesList.GetItems().size();
  bool reverseOrder = false;
  // if there are already entries in the cache, then
  // put the new ones in the top, so they show up first
  if(size)
    {
    reverseOrder = true;
    }

  // all the current values are not new any more
  std::set<fltk::PropertyItem*> items = m_CacheEntriesList.GetItems();
  for(std::set<fltk::PropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    fltk::PropertyItem* item = *i;
    item->m_NewValue = false;
    }
  // Prepare to add rows to the FLTK scroll/pack
  propertyListPack->clear();
  propertyListPack->begin();

  const cmCacheManager::CacheEntryMap &cache =
    cmCacheManager::GetInstance()->GetCacheMap();
  if(cache.size() == 0)
    {
    m_OKButton->deactivate();
    }
  else
    {
    m_OKButton->activate();
    }



  for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
      i != cache.end(); ++i)
    {
    const char* key = i->first.c_str();
    const cmCacheManager::CacheEntry& value = i->second;
    
    switch(value.m_Type )
      {
      case cmCacheManager::BOOL:
        if(cmSystemTools::IsOn(value.m_Value.c_str()))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         value.m_HelpString.c_str(),
                                         fltk::PropertyList::CHECKBOX,"",
                                         reverseOrder);
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         value.m_HelpString.c_str(),
                                         fltk::PropertyList::CHECKBOX,"",
                                         reverseOrder);
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::PATH,"",
                                       reverseOrder);
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::FILE,"",
                                       reverseOrder);
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       fltk::PropertyList::EDIT,"",
                                       reverseOrder);
        break;
      case cmCacheManager::INTERNAL:
        // These entries should not be seen by the user
        m_CacheEntriesList.RemoveProperty(key);
        break;
      }

    }

  // Add the old entry to the end of the pack
  for(std::set<fltk::PropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    fltk::PropertyItem* item = *i;
    if( !(item->m_NewValue) )
      {
      new fltk::PropertyItemRow( item ); // GUI of the old property row
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
      if( item->m_Dirty )
        {
        m_CacheEntriesList.SetDirty();
        }
    }

}




/**
 * Load Recent Directories
 */
void
CMakeSetupGUIImplementation
::LoadRecentDirectories( void )
{
  std::string home = getenv("HOME");
  std::string filename = home + "/.cmakerc";
  
  std::ifstream input;
  input.open(filename.c_str());
  
  if( input.fail() ) 
  {
    // probably the file doesn't exist
    return;
  }
  
  m_RecentBinaryDirectories.clear();
  m_RecentSourceDirectories.clear();

  std::string key;
  std::string onedirectory;

  while( !input.eof() )
  {
    input >> key;
    
    if( input.eof() ) break;

    if( key == "MostRecentSource" )
    {
      input >> onedirectory;
      m_WhereSource = onedirectory;
      sourcePathTextInput->value( m_WhereSource.c_str() );
    } else
    if( key == "MostRecentBinary" )
    {
      input >> onedirectory;
      m_WhereBuild = onedirectory;
      binaryPathTextInput->value( m_WhereBuild.c_str() );
    } else
    if( key == "Binary" )
    {
      input >> onedirectory;
      // insert is only done if the directory doesn't exist
      m_RecentBinaryDirectories.insert( onedirectory );
      recentBinaryDirectoriesBrowser->add(
                      (onedirectory.c_str()),
                      (void*)(onedirectory.c_str()) );
    } else
    if( key == "Source" )
    {
      input >> onedirectory;
      // insert is only done if the directory doesn't exist
      m_RecentSourceDirectories.insert( onedirectory );
      recentSourceDirectoriesBrowser->add(
                      (onedirectory.c_str()),
                      (void*)(onedirectory.c_str()) );
    }

  }

  input.close();
}



/**
 * Save Recent Directories
 */
void
CMakeSetupGUIImplementation
::SaveRecentDirectories( void )
{
  std::string home = getenv("HOME");

  if( home.empty() )
  {
    return;
  }
  
  std::string filename = home + "/.cmakerc";
  
  std::ofstream output;
  output.open(filename.c_str());

  output << "MostRecentBinary " << m_WhereBuild  << std::endl;
  output << "MostRecentSource " << m_WhereSource << std::endl;

  // Save Recent binary directories
  std::set< std::string >::iterator bindir = 
                m_RecentBinaryDirectories.begin();

  while( bindir != m_RecentBinaryDirectories.end() )
  {
    output << "Binary " << *bindir << std::endl;
    bindir++;
  }


  // Save Recent source directories
  std::set< std::string >::iterator srcdir = 
                m_RecentSourceDirectories.begin();

  while( srcdir != m_RecentSourceDirectories.end() )
  {
    output << "Source " << *srcdir << std::endl;
    srcdir++;
  }

}


/**
 * Show Recent Binary Directories
 */
void
CMakeSetupGUIImplementation
::ShowRecentBinaryDirectories( void )
{
  if( recentBinaryDirectoriesBrowser->size() )
    {
    recentBinaryDirectoriesBrowser->Fl_Widget::show();
    }
}


/**
 * Show Recent Source Directories
 */
void
CMakeSetupGUIImplementation
::ShowRecentSourceDirectories( void )
{
  if( recentSourceDirectoriesBrowser->size() )
    {
    recentSourceDirectoriesBrowser->Fl_Widget::show();
    }
}


/**
 * Select one Recent Binary Directory
 */
void
CMakeSetupGUIImplementation
::SelectOneRecentBinaryDirectory( void )
{
  const int selected = recentBinaryDirectoriesBrowser->value();
  if( selected == 0 )
  {
    return;
  }
  
  m_WhereBuild = static_cast<char *>(
           recentBinaryDirectoriesBrowser->data( selected ));
  binaryPathTextInput->value( m_WhereBuild.c_str() );
  recentBinaryDirectoriesBrowser->Fl_Widget::hide();
}


/**
 * Select one Recent Source Directory
 */
void
CMakeSetupGUIImplementation
::SelectOneRecentSourceDirectory( void )
{
  const int selected = recentSourceDirectoriesBrowser->value();
  if( selected == 0 )
  {
    return;
  }
  m_WhereSource =  static_cast< char * >(
          recentSourceDirectoriesBrowser->data( selected ));
  sourcePathTextInput->value( m_WhereSource.c_str() );
  recentSourceDirectoriesBrowser->Fl_Widget::hide();
}



/**
 * Update List of Recent Directories
 */
void
CMakeSetupGUIImplementation
::UpdateListOfRecentDirectories( void )
{

  // Update Recent binary directories
  // insert is only done if the directory doesn't exist
  m_RecentBinaryDirectories.insert( m_WhereBuild );
  
  // Update Recent source directories
  // insert is only done if the directory doesn't exist
  m_RecentSourceDirectories.insert( m_WhereSource );

}





/**
 * Clicked on Configure Button
 */
void
CMakeSetupGUIImplementation
::ClickOnConfigure( void )
{
  this->RunCMake(false);
}




/**
 * Clicked on OK Button
 */
void
CMakeSetupGUIImplementation
::ClickOnOK( void )
{
  m_CacheEntriesList.ClearDirty();
  this->RunCMake(true);
  cmMakefileGenerator::UnRegisterGenerators();
  this->Close();
}




/**
 * Clicked on Cancel Button
 */
void
CMakeSetupGUIImplementation
::ClickOnCancel( void )
{
  if(m_CacheEntriesList.IsDirty())
    {
    int userWantsExitEvenThoughOptionsHaveChanged = 
      fl_ask("You have changed options but not rebuilt, \n"
		  "are you sure you want to exit?");
    if( userWantsExitEvenThoughOptionsHaveChanged )
      {
      this->Close();
      }
    }
  else
    {
    this->Close();
    }

}





