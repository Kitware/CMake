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

cmCursesMainForm::cmCursesMainForm(std::vector<std::string> const& args) :
  m_Args(args)
{
  m_Fields = 0;
  m_Height = 0;
  m_Entries = 0;
  m_AdvancedMode = false;
  m_NumberOfVisibleEntries = 0;
  m_OkToGenerate = false;
  m_HelpMessage.push_back("Welcome to ccmake, curses based user interface for CMake.");
  m_HelpMessage.push_back("");
  m_HelpMessage.push_back(s_ConstHelpMessage);
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
}

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

void cmCursesMainForm::InitializeUI()
{

  // Get the cache entries.
  const cmCacheManager::CacheEntryMap &cache = 
    cmCacheManager::GetInstance()->GetCacheMap();

  std::vector<cmCursesCacheEntryComposite*>* newEntries =
    new std::vector<cmCursesCacheEntryComposite*>;
  newEntries->reserve(cache.size());

  // Count non-internal and non-static entries
  int count=0;
  for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
      i != cache.end(); ++i)
    {
    const cmCacheManager::CacheEntry& value = i->second;
    if ( value.m_Type != cmCacheManager::INTERNAL &&
	 value.m_Type != cmCacheManager::STATIC )
      {
      ++count;
      }
    }

  cmCursesCacheEntryComposite* comp;
  if ( count == 0 )
    {
    // If cache is empty, display a label saying so and a
    // dummy entry widget (does not respond to input)
    comp = new cmCursesCacheEntryComposite("EMPTY CACHE");
    comp->m_Entry = new cmCursesDummyWidget(1, 1, 1, 1);
    newEntries->push_back(comp);
    }
  else
    {
    // Create the composites.

    // First add entries which are new
    for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
	i != cache.end(); ++i)
      {
      const char* key = i->first.c_str();
      const cmCacheManager::CacheEntry& value = i->second;
      if ( value.m_Type == cmCacheManager::INTERNAL || 
	   value.m_Type == cmCacheManager::STATIC )
	{
	continue;
	}

      if (!this->LookForCacheEntry(key))
	{
	newEntries->push_back(new cmCursesCacheEntryComposite(key, value,
							      true));
	m_OkToGenerate = false;
	}
      }

    // then add entries which are old
    for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
	i != cache.end(); ++i)
      {
      const char* key = i->first.c_str();
      const cmCacheManager::CacheEntry& value = i->second;
      if ( value.m_Type == cmCacheManager::INTERNAL || 
	   value.m_Type == cmCacheManager::STATIC )
	{
	continue;
	}

      if (this->LookForCacheEntry(key))
	{
	newEntries->push_back(new cmCursesCacheEntryComposite(key, value,
							      false));
	}
      }
    }
  
  if (m_Entries)
    {
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      delete *it;
      }
    }
  delete m_Entries;
  m_Entries = newEntries;
  
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
    m_NumberOfVisibleEntries = 0;
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      if (!m_AdvancedMode && cmCacheManager::GetInstance()->IsAdvanced(
	(*it)->GetValue()))
	{
	continue;
	}
      m_NumberOfVisibleEntries++;
      }
    }

  m_Fields = new FIELD*[3*m_NumberOfVisibleEntries+1];

  int j=0;
  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    if (!m_AdvancedMode && cmCacheManager::GetInstance()->IsAdvanced(
      (*it)->GetValue()))
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
    if ( cw->GetType() == cmCacheManager::STRING ||
	 cw->GetType() == cmCacheManager::PATH   ||
	 cw->GetType() == cmCacheManager::FILEPATH )
      {
      cmCursesStringWidget* sw = static_cast<cmCursesStringWidget*>(cw);
      sw->SetInEdit(false);
      }
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }
  if ( width < cmCursesMainForm::MIN_WIDTH  || 
       height < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }

  height -= 6;
  m_Height = height;

  if (m_AdvancedMode)
    {
    m_NumberOfVisibleEntries = m_Entries->size();
    }
  else
    {
    m_NumberOfVisibleEntries = 0;
    std::vector<cmCursesCacheEntryComposite*>::iterator it;
    for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
      {
      if (!m_AdvancedMode && cmCacheManager::GetInstance()->IsAdvanced(
	(*it)->GetValue()))
	{
	continue;
	}
      m_NumberOfVisibleEntries++;
      }
    }

  bool isNewPage;
  int i=0;
  std::vector<cmCursesCacheEntryComposite*>::iterator it;
  for (it = m_Entries->begin(); it != m_Entries->end(); ++it)
    {
    if (!m_AdvancedMode && cmCacheManager::GetInstance()->IsAdvanced(
      (*it)->GetValue()))
      {
      continue;
      }
    int row = (i % height) + 1;  
    int page = (i / height) + 1;
    isNewPage = ( page > 1 ) && ( row == 1 );

    (*it)->m_Label->Move(left, top+row-1, isNewPage);
    (*it)->m_IsNewLabel->Move(left+32, top+row-1, false);
    (*it)->m_Entry->Move(left+33, top+row-1, false);
    i++;
    }

  m_Form = new_form(m_Fields);
  post_form(m_Form);
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
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }
  char firstLine[512], secondLine[512];
  if (m_OkToGenerate)
    {
    sprintf(firstLine,  "C)onfigure                 G)enerate and Exit            H)elp");
    }
  else
    {
    sprintf(firstLine,  "C)onfigure                                               H)elp");
    }
  if (m_AdvancedMode)
    {
    sprintf(secondLine, "Q)uit Without Generating   T)oggle Advanced Mode (On)");
    }
  else
    {
    sprintf(secondLine, "Q)uit Without Generating   T)oggle Advanced Mode (Off)");
    }

  curses_move(y-2,0);
  printw(firstLine);
  curses_move(y-1,0);
  printw(secondLine);
  pos_form_cursor(m_Form);
  
}

// Print the key of the current entry and the CMake version
// on the status bar. Designed for a width of 80 chars.
void cmCursesMainForm::UpdateStatusBar()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    curses_clear();
    curses_move(0,0);
    printw("Window is too small. A size of at least %dx%d is required.",
	   cmCursesMainForm::MIN_WIDTH, cmCursesMainForm::MIN_HEIGHT);
    touchwin(stdscr); 
    wrefresh(stdscr); 
    return;
    }

  FIELD* cur = current_field(m_Form);
  int index = field_index(cur);
  cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
    m_Fields[index-2]));
  const char* curField = lbl->GetValue();

  // We want to display this on the right
  char help[128];
  const char* helpString;
  cmCacheManager::CacheEntry *entry = 
    cmCacheManager::GetInstance()->GetCacheEntry(curField);
  if (entry)
    {
    helpString = entry->m_HelpString.c_str();
    if (strlen(helpString) > 127)
      {
      sprintf(help,"%127s", helpString);
      }
    else
      {
      sprintf(help,"%s", helpString);
      }
    }
  else
    {
    sprintf(help," ");
    }


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

  char version[cmCursesMainForm::MAX_WIDTH];
  char vertmp[128];
  sprintf(vertmp,"CMake Version %d.%d", cmMakefile::GetMajorVersion(),
	  cmMakefile::GetMinorVersion());
  int sideSpace = (width-strlen(vertmp));
  for(i=0; i<sideSpace; i++) { version[i] = ' '; }
  sprintf(version+sideSpace, "%s", vertmp);
  version[width] = '\0';

  curses_move(y-4,0);
  attron(A_STANDOUT);
  printw(bar);
  attroff(A_STANDOUT);  
  curses_move(y-3,0);
  printw(version);
  pos_form_cursor(m_Form);
}

void cmCursesMainForm::RunCMake(bool generateMakefiles)
{

  int x,y;
  getmaxyx(stdscr, y, x);

  curses_clear();
  curses_move(1,1);
  printw("Running CMake, please wait.");
  touchwin(stdscr);
  refresh();
  endwin();
  // always save the current gui values to disk
  this->FillCacheManagerFromUI();
  cmCacheManager::GetInstance()->SaveCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());

  // create a cmake object
  cmake make;
  // create the arguments for the cmake object
  std::string whereCMake = cmSystemTools::GetProgramPath(m_Args[0].c_str());
  whereCMake += "/cmake";

  m_Args[0] = whereCMake;

  // Get rid of previous errors
  m_Errors = std::vector<std::string>();

  // run the generate process
  m_OkToGenerate = true;
  if(make.Generate(m_Args, generateMakefiles) != 0 || !m_Errors.empty())
    {
    m_OkToGenerate = false;
    cmSystemTools::ResetErrorOccuredFlag();
    int x,y;
    getmaxyx(stdscr, y, x);
    cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_Errors,
								"Errors which during last pass.");
    CurrentForm = msgs;
    msgs->Render(1,1,x,y);
    msgs->HandleInput();
    CurrentForm = this;
    this->Render(1,1,x,y);
    }

  initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
			  KEY_DOWN*/ 
   
  this->InitializeUI();
  this->Render(1, 1, x, y);
  
}

void cmCursesMainForm::AddError(const char* message, const char* title)
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
  
  cmCacheManager::GetInstance()->GetCacheMap();
  int size = m_Entries->size();
  for(int i=0; i < size; i++)
    {
    cmCacheManager::CacheEntry *entry = 
      cmCacheManager::GetInstance()->GetCacheEntry(
	(*m_Entries)[i]->m_Key.c_str());
    if (entry)
      {
      tmpString = (*m_Entries)[i]->m_Entry->GetValue();
      // Remove trailing spaces
      entry->m_Value = tmpString.substr(0,tmpString.find_last_not_of(" ")+1);
      }
    }
}

void cmCursesMainForm::HandleInput()
{
  if (!m_Form)
    {
    return;
    }

  FIELD* currentField;
  cmCursesWidget* currentWidget;
  while(1)
    {
    this->UpdateStatusBar();
    this->PrintKeys();
    int key = getch();

    currentField = current_field(m_Form);
    currentWidget = reinterpret_cast<cmCursesWidget*>(field_userptr(
      currentField));

    if (!currentWidget || !currentWidget->HandleInput(key, m_Form, stdscr))
      {
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
	int index = field_index(cur);
	if ( index == 3*m_NumberOfVisibleEntries-1 )
	  {
	  continue;
	  }
	if (new_page(m_Fields[index+1]))
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
	int index = field_index(cur);
	if ( index == 2 )
	  {
	  continue;
	  }
	if ( new_page(m_Fields[index-2]) )
	  {
	  form_driver(m_Form, REQ_PREV_PAGE);
	  set_current_field(m_Form, m_Fields[index-3]);
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
	this->RunCMake(false);
	}
      // display help
      else if ( key == 'h' )
	{
	int x,y;
	getmaxyx(stdscr, y, x);

	FIELD* cur = current_field(m_Form);
	int index = field_index(cur);
	cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
	  m_Fields[index-2]));
	const char* curField = lbl->GetValue();
	const char* helpString=0;
	cmCacheManager::CacheEntry *entry = 
	  cmCacheManager::GetInstance()->GetCacheEntry(curField);
	if (entry)
	  {
	  helpString = entry->m_HelpString.c_str();
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
	int x,y;
	getmaxyx(stdscr, y, x);
	cmCursesLongMessageForm* msgs = new cmCursesLongMessageForm(m_Errors,
								    "Errors which during last pass.");
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
	int x,y;
	getmaxyx(stdscr, y, x);
	this->RePost();
	this->Render(1, 1, x, y);
	}
      // generate and exit
      else if ( key == 'g' )
	{
	if ( m_OkToGenerate )
	  {
	  this->RunCMake(true);
	  break;
	  }
	}
      // delete cache entry
      else if ( key == 'd' )
	{
	FIELD* cur = current_field(m_Form);
	int index = field_index(cur);

	// make the next or prev. current field after deletion
	// each entry consists of fields: label, isnew, value
	// therefore, the label field for the prev. entry is index-5
	// and the label field for the next entry is index+1
	// (index always corresponds to the value field)
	FIELD* nextCur;
	if ( index == 2 )
	  {
	  nextCur=0;
	  }
	else if ( index == 3*m_NumberOfVisibleEntries-1 )
	  {
	  nextCur = m_Fields[index-5];
	  }
	else
	  {
	  nextCur = m_Fields[index+1];
	  }

	// Get the label widget
	// each entry consists of fields: label, isnew, value
	// therefore, the label field for the is index-2
	// (index always corresponds to the value field)
	cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
	  m_Fields[index-2]));
	cmCacheManager::GetInstance()->RemoveCacheEntry(lbl->GetValue());

	std::string nextVal;
	if (nextCur)
	  {
	  nextVal = (reinterpret_cast<cmCursesWidget*>(field_userptr(nextCur))->GetValue());
	  }

	int x,y;
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
"You can also delete an option by pressing 'd'\n\n"
"Commands:\n"
" q : quit ccmake without generating build files\n"
" h : help, shows this screen\n"
" c : process the configuration files with the current options\n"
" g : generate build files and exit, only available when there are no "
"new options and no errors have been detected during last configuration.\n"
" l : shows last errors\n"
" t : toggles advanced mode. In normal mode, only the most important options are shown. In advanced mode, all options are shown. We recommend using normal mode unless you are an expert.\n";


