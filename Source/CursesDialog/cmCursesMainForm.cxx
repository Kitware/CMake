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
#include "../cmCacheManager.h"
#include "../cmSystemTools.h"
#include "../cmake.h"
#include "cmCursesMainForm.h"
#include "cmCursesStringWidget.h"
#include "cmCursesLabelWidget.h"
#include "cmCursesBoolWidget.h"
#include "cmCursesPathWidget.h"
#include "cmCursesFilePathWidget.h"
#include "cmCursesDummyWidget.h"
#include "cmCursesCacheEntryComposite.h"
#include "cmCursesLongMessageForm.h"


inline int ctrl(int z)
{
    return (z&037);
} 

cmCursesMainForm::cmCursesMainForm(std::vector<std::string> const& args,
                                   int initWidth) :
  m_Args(args), m_InitialWidth(initWidth)
{
  m_NumberOfPages = 0;
  m_Fields = 0;
  m_Entries = 0;
  m_AdvancedMode = false;
  m_NumberOfVisibleEntries = 0;
  m_OkToGenerate = false;
  m_HelpMessage.push_back("Welcome to ccmake, curses based user interface for CMake.");
  m_HelpMessage.push_back("");
  m_HelpMessage.push_back(s_ConstHelpMessage);
  m_CMakeInstance = new cmake;

  // create the arguments for the cmake object
  std::string whereCMake = cmSystemTools::GetProgramPath(m_Args[0].c_str());
  whereCMake += "/cmake";
  m_Args[0] = whereCMake;
  m_CMakeInstance->SetArgs(m_Args);
  m_CMakeInstance->SetCMakeCommand(whereCMake.c_str());
}

cmCursesMainForm::~cmCursesMainForm()
{
  if (m_Form)
    {
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }
  delete[] m_Fields;

  // Clean-up composites
  if (m_Entries)
    {
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      delete *it;
      }
    }
  delete m_Entries;
  if (this->m_CMakeInstance)
    {
    delete this->m_CMakeInstance;
    this->m_CMakeInstance = 0;
    }  
}

// See if a cache entry is in the list of entries in the ui.
bool cmCursesMainForm::LookForCacheEntry(const char* key)
{
  if (!key || !m_Entries)
    {
    return false;
    }

  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    if (!strcmp(key, (*it)->m_Key.c_str()))
      {
      return true;
      }
    }
  
  return false;
}

// Create new cmCursesCacheEntryComposite entries from the cache
void cmCursesMainForm::InitializeUI()
{
  // Create a vector of cmCursesCacheEntryComposite's
  // which contain labels, entries and new entry markers
  std::vector<cmCursesCacheEntryComposite*>* newEntries =
    new std::vector<cmCursesCacheEntryComposite*>;
  newEntries->reserve(this->m_CMakeInstance->GetCacheManager()->GetSize());

  // Count non-internal and non-static entries
  int count=0;
  for(cmCacheManager::CacheIterator i = 
        this->m_CMakeInstance->GetCacheManager()->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    if ( i.GetType() != cmCacheManager::INTERNAL &&
         i.GetType() != cmCacheManager::STATIC  &&
         i.GetType() != cmCacheManager::UNINITIALIZED)
      {
      ++count;
      }
    }

  int entrywidth = m_InitialWidth - 35;

  cmCursesCacheEntryComposite* comp;
  if ( count == 0 )
    {
    // If cache is empty, display a label saying so and a
    // dummy entry widget (does not respond to input)
    comp = new cmCursesCacheEntryComposite("EMPTY CACHE", 30, 30);
    comp->m_Entry = new cmCursesDummyWidget(1, 1, 1, 1);
    newEntries->push_back(comp);
    }
  else
    {
    // Create the composites.

    // First add entries which are new
    for(cmCacheManager::CacheIterator i = 
          this->m_CMakeInstance->GetCacheManager()->NewIterator();
        !i.IsAtEnd(); i.Next())
      {
      const char* key = i.GetName();
      if ( i.GetType() == cmCacheManager::INTERNAL || 
           i.GetType() == cmCacheManager::STATIC ||
           i.GetType() == cmCacheManager::UNINITIALIZED )
        {
        continue;
        }

      if (!this->LookForCacheEntry(key))
        {
        newEntries->push_back(new cmCursesCacheEntryComposite(key, i,
                                                              true, 30,
                                                              entrywidth));
        m_OkToGenerate = false;
        }
      }

    // then add entries which are old
    for(cmCacheManager::CacheIterator i = 
          this->m_CMakeInstance->GetCacheManager()->NewIterator();
        !i.IsAtEnd(); i.Next())
      {
      const char* key = i.GetName();
      if ( i.GetType() == cmCacheManager::INTERNAL || 
           i.GetType() == cmCacheManager::STATIC ||
           i.GetType() == cmCacheManager::UNINITIALIZED )
        {
        continue;
        }

      if (this->LookForCacheEntry(key))
        {
        newEntries->push_back(new cmCursesCacheEntryComposite(key, i,
                                                              false, 30,
                                                              entrywidth));
        }
      }
    }
  
  // Clean old entries
  if (m_Entries)
    {
    // Have to call delete on each pointer
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      delete *it;
      }
    }
  delete m_Entries;
  m_Entries = newEntries;
  
  // Compute fields from composites
  this->RePost();
}


void cmCursesMainForm::RePost()
{
  // Create the fields to be passed to the form.
  if (m_Form)
    {
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }
  delete[] m_Fields;

  if (m_AdvancedMode)
    {
    m_NumberOfVisibleEntries = m_Entries->size();
    }
  else
    {
    // If normal mode, count only non-advanced entries
    m_NumberOfVisibleEntries = 0;
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      cmCacheManager::CacheIterator mit = 
        this->m_CMakeInstance->GetCacheManager()->GetCacheIterator((*it)->GetValue());
      if (mit.IsAtEnd() || !m_AdvancedMode && mit.GetPropertyAsBool("ADVANCED"))
        {
        continue;
        }
      m_NumberOfVisibleEntries++;
      }
    }

  // Assign the fields: 3 for each entry: label, new entry marker
  // ('*' or ' ') and entry widget
  m_Fields = new FIELD*[3*m_NumberOfVisibleEntries+1];

  // Assign fields
  int j=0;
  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    cmCacheManager::CacheIterator mit = 
      this->m_CMakeInstance->GetCacheManager()->GetCacheIterator((*it)->GetValue());
    if (mit.IsAtEnd() || !m_AdvancedMode && mit.GetPropertyAsBool("ADVANCED"))
      {
      continue;
      }
    m_Fields[3*j]    = (*it)->m_Label->m_Field;
    m_Fields[3*j+1]  = (*it)->m_IsNewLabel->m_Field;
    m_Fields[3*j+2]  = (*it)->m_Entry->m_Field;
    j++;
    }

  // Has to be null terminated.
  m_Fields[3*m_NumberOfVisibleEntries] = 0;
}

void cmCursesMainForm::Render(int left, int top, int width, int height)
{

  if (m_Form)
    {
    FIELD* currentField = current_field(m_Form);
    cmCursesWidget* cw = reinterpret_cast<cmCursesWidget*>
      (field_userptr(currentField));
    // If in edit mode, get out of it
    if ( cw->GetType() == cmCacheManager::STRING ||
         cw->GetType() == cmCacheManager::PATH   ||
         cw->GetType() == cmCacheManager::FILEPATH )
      {
      cmCursesStringWidget* sw = static_cast<cmCursesStringWidget*>(cw);
      sw->SetInEdit(false);
      }
    // Delete the previous form
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }

  // Wrong window size
  if ( width < cmCursesMainForm::MIN_WIDTH  || 
       width < m_InitialWidth               ||
       height < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }

  // Leave room for toolbar
  height -= 7;

  if (m_AdvancedMode)
    {
    m_NumberOfVisibleEntries = m_Entries->size();
    }
  else
    {
    // If normal, display only non-advanced entries
    m_NumberOfVisibleEntries = 0;
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      cmCacheManager::CacheIterator mit = 
        this->m_CMakeInstance->GetCacheManager()->GetCacheIterator((*it)->GetValue());
      if (mit.IsAtEnd() || !m_AdvancedMode && mit.GetPropertyAsBool("ADVANCED"))
        {
        continue;
        }
      m_NumberOfVisibleEntries++;
      }
    }

  // Re-adjust the fields according to their place
  bool isNewPage;
  int i=0;
  m_NumberOfPages = 1;
  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    cmCacheManager::CacheIterator mit = 
      this->m_CMakeInstance->GetCacheManager()->GetCacheIterator((*it)->GetValue());
    if (mit.IsAtEnd() || !m_AdvancedMode && mit.GetPropertyAsBool("ADVANCED"))
      {
      continue;
      }
    int row = (i % height) + 1;  
    int page = (i / height) + 1;
    isNewPage = ( page > 1 ) && ( row == 1 );

    if (isNewPage)
      {
      m_NumberOfPages++;
      }
    (*it)->m_Label->Move(left, top+row-1, isNewPage);
    (*it)->m_IsNewLabel->Move(left+32, top+row-1, false);
    (*it)->m_Entry->Move(left+33, top+row-1, false);
    (*it)->m_Entry->SetPage(m_NumberOfPages);
    i++;
    }

  // Post the form
  m_Form = new_form(m_Fields);
  post_form(m_Form);
  // Update toolbar
  this->UpdateStatusBar();
  this->PrintKeys();

  touchwin(stdscr); 
  refresh();
}

void cmCursesMainForm::PrintKeys()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       x < m_InitialWidth               ||
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }

  // Give the current widget (if it exists), a chance to print keys
  cmCursesWidget* cw = 0;
  if (m_Form)
    {
    FIELD* currentField = current_field(m_Form);
    cw = reinterpret_cast<cmCursesWidget*>(field_userptr(currentField));
    }

  if (cw)
    {
    cw->PrintKeys();
    }
  
//    {
//    }
//  else
//    {
    char firstLine[512], secondLine[512], thirdLine[512];
    if (m_OkToGenerate)
      {
      sprintf(firstLine,  "Press [c] to configure     Press [g] to generate and exit");
      }
    else
      {
      sprintf(firstLine,  "Press [c] to configure                                   ");
      }
    if (m_AdvancedMode)
      {
      sprintf(thirdLine,  "Press [t] to toggle advanced mode (Currently On)");
      }
    else
      {
      sprintf(thirdLine,  "Press [t] to toggle advanced mode (Currently Off)");
      }
    
    sprintf(secondLine, "Press [h] for help         Press [q] to quit without generating");


    curses_move(y-4,0);
    char fmt[] = "Press [enter] to edit option";
    printw(fmt);
    curses_move(y-3,0);
    printw(firstLine);
    curses_move(y-2,0);
    printw(secondLine);
    curses_move(y-1,0);
    printw(thirdLine);

    if (cw)
      {
      sprintf(firstLine, "Page %d of %d", cw->GetPage(), m_NumberOfPages);
      curses_move(0,65-strlen(firstLine)-1);
      printw(firstLine);
      }
//    }

  pos_form_cursor(m_Form);
  
}

// Print the key of the current entry and the CMake version
// on the status bar. Designed for a width of 80 chars.
void cmCursesMainForm::UpdateStatusBar()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  // If window size is too small, display error and return
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       x < m_InitialWidth               ||
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    curses_clear();
    curses_move(0,0);
    char fmt[] = "Window is too small. A size of at least %dx%d is required.";
    printw(fmt,
           (cmCursesMainForm::MIN_WIDTH < m_InitialWidth ?
            m_InitialWidth : cmCursesMainForm::MIN_WIDTH), 
           cmCursesMainForm::MIN_HEIGHT);
    touchwin(stdscr); 
    wrefresh(stdscr); 
    return;
    }

  // Get the key of the current entry
  FIELD* cur = current_field(m_Form);
  int findex = field_index(cur);
  cmCursesWidget* lbl = 0;
  if ( findex >= 0 )
    {
    lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(m_Fields[findex-2]));
    }
  char help[128] = "";
  const char* curField = "";
  if ( lbl )
    {
    curField = lbl->GetValue();
    
    // Get the help string of the current entry
    // and add it to the help string
    cmCacheManager::CacheIterator it = 
      this->m_CMakeInstance->GetCacheManager()->GetCacheIterator(curField);
    if (!it.IsAtEnd())
      {
      const char* hs = it.GetProperty("HELPSTRING");
      if ( hs )
        {
        strncpy(help, hs, 127);
        help[127] = '\0';
        }
      else
        {
        help[0] = 0;
        }
      }
    else
      {
      sprintf(help," ");
      }
    }

  // Join the key, help string and pad with spaces
  // (or truncate) as necessary
  char bar[cmCursesMainForm::MAX_WIDTH];
  int i, curFieldLen = strlen(curField);
  int helpLen = strlen(help);

  int width;
  if (x < cmCursesMainForm::MAX_WIDTH )
    {
    width = x;
    }
  else
    {
    width = cmCursesMainForm::MAX_WIDTH;
    }

  if (curFieldLen >= width)
    {
    strncpy(bar, curField, width);
    }
  else
    {
    strcpy(bar, curField);
    bar[curFieldLen] = ':';
    bar[curFieldLen+1] = ' ';
    if (curFieldLen + helpLen + 2 >= width)
      {
      strncpy(bar+curFieldLen+2, help, width
              - curFieldLen - 2);
      }
    else
      {
      strcpy(bar+curFieldLen+2, help);
      for(i=curFieldLen+helpLen+2; i < width; ++i) 
        { 
        bar[i] = ' '; 
        }
      }
    }

  bar[width] = '\0';

  // Display CMake version info on the next line
  // We want to display this on the right
  char version[cmCursesMainForm::MAX_WIDTH];
  char vertmp[128];
  sprintf(vertmp,"CMake Version %d.%d - %s", cmake::GetMajorVersion(),
          cmake::GetMinorVersion(),cmake::GetReleaseVersion());
  int sideSpace = (width-strlen(vertmp));
  for(i=0; i<sideSpace; i++) { version[i] = ' '; }
  sprintf(version+sideSpace, "%s", vertmp);
  version[width] = '\0';

  // Now print both lines
  curses_move(y-5,0);
  attron(A_STANDOUT);
  printw(bar);
  attroff(A_STANDOUT);  
  curses_move(y-4,0);
  printw(version);
  pos_form_cursor(m_Form);
}

int cmCursesMainForm::Configure()
{

  int xi,yi;
  getmaxyx(stdscr, yi, xi);

  curses_clear();
  curses_move(1,1);
  touchwin(stdscr);
  refresh();
  endwin();
  std::cerr << "Configuring, please wait...\n\r";


  // always save the current gui values to disk
  this->FillCacheManagerFromUI();
  this->m_CMakeInstance->GetCacheManager()->SaveCache(
    cmSystemTools::GetCurrentWorkingDirectory().c_str());

  
  // Get rid of previous errors
  m_Errors = std::vector<std::string>();

  // run the generate process
  m_OkToGenerate = true;
  int retVal = this->m_CMakeInstance->Configure();

  initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
                          KEY_DOWN*/ 

  if( retVal != 0 || !m_Errors.empty())
    {
    // see if there was an error
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      m_OkToGenerate = false;
      }
    // reset error condition
    cmSystemTools::ResetErrorOccuredFlag();
    int xx,yy;
    getmaxyx(stdscr, yy, xx);
    cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_Errors,
                                                                "Errors occurred during the last pass.");
    CurrentForm = msgs;
    msgs->Render(1,1,xx,yy);
    msgs->HandleInput();
    // If they typed the wrong source directory, we report
    // an error and exit
    if ( retVal == -2 )
      {
      return retVal;
      }
    CurrentForm = this;
    this->Render(1,1,xx,yy);
    }
   
  this->InitializeUI();
  this->Render(1, 1, xi, yi);
  
  return 0;
}

int cmCursesMainForm::Generate()
{
  int xi,yi;
  getmaxyx(stdscr, yi, xi);

  curses_clear();
  curses_move(1,1);
  touchwin(stdscr);
  refresh();
  endwin();
  std::cerr << "Generating, please wait...\n\r";

  // Get rid of previous errors
  m_Errors = std::vector<std::string>();

  // run the generate process
  int retVal = this->m_CMakeInstance->Generate();

  initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
                          KEY_DOWN*/ 

  if( retVal != 0 || !m_Errors.empty())
    {
    // see if there was an error
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      m_OkToGenerate = false;
      }
    // reset error condition
    cmSystemTools::ResetErrorOccuredFlag();
    int xx,yy;
    getmaxyx(stdscr, yy, xx);
    cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_Errors,
                                                                "Errors occurred during the last pass.");
    CurrentForm = msgs;
    msgs->Render(1,1,xx,yy);
    msgs->HandleInput();
    // If they typed the wrong source directory, we report
    // an error and exit
    if ( retVal == -2 )
      {
      return retVal;
      }
    CurrentForm = this;
    this->Render(1,1,xx,yy);
    }
  
  this->InitializeUI();
  this->Render(1, 1, xi, yi);
  
  return 0;
}

void cmCursesMainForm::AddError(const char* message, const char*)
{
  m_Errors.push_back(message);
}

void cmCursesMainForm::RemoveEntry(const char* value)
{
  if (!value)
    {
    return;
    }

  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    const char* val = (*it)->GetValue();
    if (  val && !strcmp(value, val) )
      {
      m_Entries->erase(it);
      break;
      }
    }
}

// copy from the list box to the cache manager
void cmCursesMainForm::FillCacheManagerFromUI()
{ 
  std::string tmpString;
  
  int size = m_Entries->size();
  for(int i=0; i < size; i++)
    {
    cmCacheManager::CacheIterator it = 
      this->m_CMakeInstance->GetCacheManager()->GetCacheIterator(
        (*m_Entries)[i]->m_Key.c_str());
    if (!it.IsAtEnd())
      {
      tmpString = (*m_Entries)[i]->m_Entry->GetValue();

      // Remove trailing spaces, convert path to unix slashes
      std::string tmpSubString = 
        tmpString.substr(0,tmpString.find_last_not_of(" ")+1);
      if ( it.GetType() == cmCacheManager::PATH || 
           it.GetType() == cmCacheManager::FILEPATH )
        {
        cmSystemTools::ConvertToUnixSlashes(tmpSubString);
        }
      it.SetValue(tmpSubString.c_str());
      }
    }
}

void cmCursesMainForm::HandleInput()
{
  int x,y;

  if (!m_Form)
    {
    return;
    }

  FIELD* currentField;
  cmCursesWidget* currentWidget;

  char debugMessage[128];

  while(1)
    {
    this->UpdateStatusBar();
    this->PrintKeys();
    int key = getch();

    getmaxyx(stdscr, y, x);
    // If window too small, handle 'q' only
    if ( x < cmCursesMainForm::MIN_WIDTH  || 
         y < cmCursesMainForm::MIN_HEIGHT )
      {
      // quit
      if ( key == 'q' )
        {
        break;
        }
      else
        {
        continue;
        }
      }

    currentField = current_field(m_Form);
    currentWidget = reinterpret_cast<cmCursesWidget*>(field_userptr(
      currentField));

    // Ask the current widget if it wants to handle input
    bool widgetHandled;
    
    if (currentWidget)
      {
      widgetHandled = currentWidget->HandleInput(key, this, stdscr);
      if (widgetHandled)
        {
        m_OkToGenerate = false;
        this->UpdateStatusBar();
        this->PrintKeys();
        }
      }
    if (!currentWidget || !widgetHandled)
      {
      // If the current widget does not want to handle input, 
      // we handle it.
      sprintf(debugMessage, "Main form handling input, key: %d", key);
      cmCursesForm::LogMessage(debugMessage);
      // quit
      if ( key == 'q' )
        {
        break;
        }
      // if not end of page, next field otherwise next page
      // each entry consists of fields: label, isnew, value
      // therefore, the label field for the prev. entry is index-5
      // and the label field for the next entry is index+1
      // (index always corresponds to the value field)
      else if ( key == KEY_DOWN || key == ctrl('n') )
        {
        FIELD* cur = current_field(m_Form);
        int findex = field_index(cur);
        if ( findex == 3*m_NumberOfVisibleEntries-1 )
          {
          continue;
          }
        if (new_page(m_Fields[findex+1]))
          {
          form_driver(m_Form, REQ_NEXT_PAGE);
          }
        else
          {
          form_driver(m_Form, REQ_NEXT_FIELD);
          }
        }
      // if not beginning of page, previous field, otherwise previous page
      // each entry consists of fields: label, isnew, value
      // therefore, the label field for the prev. entry is index-5
      // and the label field for the next entry is index+1
      // (index always corresponds to the value field)
      else if ( key == KEY_UP || key == ctrl('p') )
        {
        FIELD* cur = current_field(m_Form);
        int findex = field_index(cur);
        if ( findex == 2 )
          {
          continue;
          }
        if ( new_page(m_Fields[findex-2]) )
          {
          form_driver(m_Form, REQ_PREV_PAGE);
          set_current_field(m_Form, m_Fields[findex-3]);
          }
        else
          {
          form_driver(m_Form, REQ_PREV_FIELD);
          }
        }
      // pg down
      else if ( key == KEY_NPAGE || key == ctrl('d') )
        {
        form_driver(m_Form, REQ_NEXT_PAGE);
        }
      // pg up
      else if ( key == KEY_PPAGE || key == ctrl('u') )
        {
        form_driver(m_Form, REQ_PREV_PAGE);
        }
      // configure
      else if ( key == 'c' )
        {
        this->Configure();
        }
      // display help
      else if ( key == 'h' )
        {
        getmaxyx(stdscr, y, x);

        FIELD* cur = current_field(m_Form);
        int findex = field_index(cur);
        cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
          m_Fields[findex-2]));
        const char* curField = lbl->GetValue();
        const char* helpString=0;
        cmCacheManager::CacheIterator it = 
          this->m_CMakeInstance->GetCacheManager()->GetCacheIterator(curField);
        if (!it.IsAtEnd())
          {
          helpString = it.GetProperty("HELPSTRING");
          }
        if (helpString)
          {
          char* message = new char[strlen(curField)+strlen(helpString)
                                  +strlen("Current option is: \n Help string for this option is: \n")+10];
          sprintf(message,"Current option is: %s\nHelp string for this option is: %s\n", curField, helpString);
          m_HelpMessage[1] = message;
          delete[] message;
          }
        else
          {
          m_HelpMessage[1] = "";
          }

        cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_HelpMessage,
                                                                    "Help.");
        CurrentForm = msgs;
        msgs->Render(1,1,x,y);
        msgs->HandleInput();
        CurrentForm = this;
        this->Render(1,1,x,y);
        }
      // display last errors
      else if ( key == 'l' )
        {
        getmaxyx(stdscr, y, x);
        cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_Errors,
                                                                    "Errors occurred during the last pass.");
        CurrentForm = msgs;
        msgs->Render(1,1,x,y);
        msgs->HandleInput();
        CurrentForm = this;
        this->Render(1,1,x,y);
        }
      // switch advanced on/off
      else if ( key == 't' )
        {
        if (m_AdvancedMode)
          {
          m_AdvancedMode = false;
          }
        else
          {
          m_AdvancedMode = true;
          }
        getmaxyx(stdscr, y, x);
        this->RePost();
        this->Render(1, 1, x, y);
        }
      // generate and exit
      else if ( key == 'g' )
        {
        if ( m_OkToGenerate )
          {
          this->Generate();
          break;
          }
        }
      // delete cache entry
      else if ( key == 'd' )
        {
        m_OkToGenerate = false;
        FIELD* cur = current_field(m_Form);
        int findex = field_index(cur);

        // make the next or prev. current field after deletion
        // each entry consists of fields: label, isnew, value
        // therefore, the label field for the prev. entry is findex-5
        // and the label field for the next entry is findex+1
        // (findex always corresponds to the value field)
        FIELD* nextCur;
        if ( findex == 2 )
          {
          nextCur=0;
          }
        else if ( findex == 3*m_NumberOfVisibleEntries-1 )
          {
          nextCur = m_Fields[findex-5];
          }
        else
          {
          nextCur = m_Fields[findex+1];
          }

        // Get the label widget
        // each entry consists of fields: label, isnew, value
        // therefore, the label field for the is findex-2
        // (findex always corresponds to the value field)
        cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
          m_Fields[findex-2]));
        this->m_CMakeInstance->GetCacheManager()->RemoveCacheEntry(lbl->GetValue());

        std::string nextVal;
        if (nextCur)
          {
          nextVal = (reinterpret_cast<cmCursesWidget*>(field_userptr(nextCur))->GetValue());
          }

        getmaxyx(stdscr, y, x);
        this->RemoveEntry(lbl->GetValue());
        this->RePost();
        this->Render(1, 1, x, y);

        if (nextCur)
          {
          // make the next or prev. current field after deletion
          nextCur = 0;
          std::vector<cmCursesCacheEntryComposite*>::iterator it;
          for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
            {
            if (nextVal == (*it)->m_Key)
              {
              nextCur = (*it)->m_Entry->m_Field;
              }
            }
          
          if (nextCur)
            {
            set_current_field(m_Form, nextCur);
            }
          }
        }
      }

    touchwin(stdscr); 
    wrefresh(stdscr); 
    }
}

void cmCursesMainForm::LoadCache(const char *)

{
  m_CMakeInstance->LoadCache(); 
  m_CMakeInstance->SetCacheArgs(m_Args);
}
  


const char* cmCursesMainForm::s_ConstHelpMessage = 
"CMake is used to configure and generate build files for software projects. "
"The basic steps for configuring a project with ccmake are as follows:\n\n"
"1. Run ccmake in the directory where you want the object and executable files to be placed (build directory). If the source directory is not the same as this build directory, you have to specify it as an argument on the command line.\n\n"
"2. When ccmake is run, it will read the configuration files and display the current build options. "
"If you have run CMake before and have updated the configuration files since then, any new entries will be displayed on top and will be marked with a *. "
"On the other hand, the first time you run ccmake, all build options will be new and will be marked as such. "
"At this point, you can modify any options (see keys below) you want to change. "
"When you are satisfied with your changes, press 'c' to have CMake process the configuration files. "
"Please note that changing some options may cause new ones to appear. These will be shown on top and will be marked with *. "
"Repeat this procedure until you are satisfied with all the options and there are no new entries. "
"At this point, a new command will appear: G)enerate and Exit. You can now hit 'g' to have CMake generate all the build files (i.e. makefiles or project files) and exit. "
"At any point during the process, you can exit ccmake with 'q'. However, this will not generate/change any build files.\n\n"
"ccmake KEYS:\n\n"
"Navigation: "
"You can use the arrow keys and page up, down to navigate the options. Alternatively, you can use the following keys: \n"
" C-n : next option\n"
" C-p : previous options\n"
" C-d : down one page\n"
" C-u : up one page\n\n"
"Editing options: "
"To change an option  press enter or return. If the current options is a boolean, this will toggle it's value. "
"Otherwise, ccmake will enter edit mode. In this mode you can edit an option using arrow keys and backspace. Alternatively, you can use the following keys:\n"
" C-b : back one character\n"
" C-f : forward one character\n"
" C-a : go to the beginning of the field\n"
" C-e : go to the end of the field\n"
" C-d : delete previous character\n"
" C-k : kill the rest of the field\n"
" Esc : Restore field (discard last changes)\n"
" Enter : Leave edit mode\n"
"You can also delete an option by pressing 'd'\n\n"
"Commands:\n"
" q : quit ccmake without generating build files\n"
" h : help, shows this screen\n"
" c : process the configuration files with the current options\n"
" g : generate build files and exit, only available when there are no "
"new options and no errors have been detected during last configuration.\n"
" l : shows last errors\n"
" t : toggles advanced mode. In normal mode, only the most important options are shown. In advanced mode, all options are shown. We recommend using normal mode unless you are an expert.\n";


