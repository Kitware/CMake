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

const int cmCursesMainForm::MIN_WIDTH  = 65;
const int cmCursesMainForm::MIN_HEIGHT = 6;
const int cmCursesMainForm::IDEAL_WIDTH = 80;
const int cmCursesMainForm::MAX_WIDTH = 512;

inline int ctrl(int z)
{
    return (z&037);
} 

cmCursesMainForm::cmCursesMainForm(const char* whereSource, 
                                   const char* whereCMake,
				   bool newCache) :
  m_WhereSource(whereSource), m_WhereCMake(whereCMake)
{
  m_Fields = 0;
  m_Window = 0;
  m_Height = 0;
  m_Entries = 0;
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

void cmCursesMainForm::InitializeUI(WINDOW* w)
{
  m_Window = w;

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
  
  // Create the fields to be passed to the form.
  if (m_Form)
    {
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }
  delete[] m_Fields;
  int size = m_Entries->size();
  m_Fields = new FIELD*[3*size+1];
  for(int j=0; j < size; j++)
    {
    m_Fields[3*j]    = (*m_Entries)[j]->m_Label->m_Field;
    m_Fields[3*j+1]  = (*m_Entries)[j]->m_IsNewLabel->m_Field;
    m_Fields[3*j+2]  = (*m_Entries)[j]->m_Entry->m_Field;
    }
  // Has to be null terminated.
  m_Fields[3*size] = 0;
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

  height -= 5;
  m_Height = height;

  int size = m_Entries->size();
  bool isNewPage;
  for(int i=0; i < size; i++)
    {
    int row = (i % height) + 1;  
    int page = (i / height) + 1;
    isNewPage = ( page > 1 ) && ( row == 1 );

    (*m_Entries)[i]->m_Label->Move(left, top+row-1, isNewPage);
    (*m_Entries)[i]->m_IsNewLabel->Move(left+32, top+row-1, false);
    (*m_Entries)[i]->m_Entry->Move(left+33, top+row-1, false);
    }
  m_Form = new_form(m_Fields);
  post_form(m_Form);
  this->UpdateStatusBar();
  this->PrintKeys();
  touchwin(m_Window); 
  refresh();
}

void cmCursesMainForm::PrintKeys()
{
  int x,y;
  getmaxyx(m_Window, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }
  char firstLine[512], secondLine[512];
  sprintf(firstLine,  "C)onfigure             G)enerate and Exit");
  sprintf(secondLine, "Q)uit                  H)elp");

  move(y-2,0);
  printw(firstLine);
  move(y-1,0);
  printw(secondLine);
  pos_form_cursor(m_Form);
  
}

// Print the key of the current entry and the CMake version
// on the status bar. Designed for a width of 80 chars.
void cmCursesMainForm::UpdateStatusBar()
{
  int x,y;
  getmaxyx(m_Window, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    move(0,0);
    printw("Window is too small. A size of at least %dx%d is required.",
	   cmCursesMainForm::MIN_WIDTH, cmCursesMainForm::MIN_HEIGHT);
    touchwin(m_Window); 
    wrefresh(m_Window); 
    return;
    }

  FIELD* cur = current_field(m_Form);
  int index = field_index(cur);
  char* curField = field_buffer(m_Fields[index-2], 0);

  char version[128];
  sprintf(version,"(CMake Version %d.%d)", cmMakefile::GetMajorVersion(),
	  cmMakefile::GetMinorVersion());

  char bar[cmCursesMainForm::MAX_WIDTH];
  int i, curFieldLen = strlen(curField);
  int versionLen = strlen(version);
  int leftLen = cmCursesMainForm::IDEAL_WIDTH - versionLen;
  if (curFieldLen >= leftLen)
    {
    strncpy(bar, curField, leftLen);
    }
  else
    {
    strcpy(bar, curField);
    for(i=curFieldLen; i < leftLen; ++i) { bar[i] = ' '; }
    }
  strcpy(bar+leftLen, version);

  if ( x < cmCursesMainForm::MAX_WIDTH )
    {
    if (x > cmCursesMainForm::IDEAL_WIDTH )
      {
      for(i=cmCursesMainForm::IDEAL_WIDTH; i < x; i++)
	{
	bar[i] = ' ';
	}
      }
    bar[x] = '\0';
    }
  else
    {
    for(i=cmCursesMainForm::IDEAL_WIDTH; 
	i < cmCursesMainForm::MAX_WIDTH-1; i++)
      {
      bar[i] = ' ';
      }
    bar[cmCursesMainForm::MAX_WIDTH-1] = '\0';
    }

  move(y-3,0);
  attron(A_STANDOUT);
  printw(bar);
  attroff(A_STANDOUT);  
  pos_form_cursor(m_Form);
}

void cmCursesMainForm::RunCMake(bool generateMakefiles)
{

  int x,y;
  getmaxyx(m_Window, y, x);
  
  endwin();
  // always save the current gui values to disk
  this->FillCacheManagerFromUI();
  cmCacheManager::GetInstance()->SaveCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());

  // create a cmake object
  cmake make;
  // create the arguments for the cmake object
  std::vector<std::string> args;
  args.push_back(m_WhereCMake);
  if (m_WhereSource != "")
    {
    std::string arg;
    arg = m_WhereSource;
    args.push_back(arg);
    }
  // run the generate process
  if(make.Generate(args, generateMakefiles) != 0)
    {
    // TODO : error message here
    cmSystemTools::ResetErrorOccuredFlag();
    }

  m_Window = initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(m_Window,TRUE); /* Use key symbols as 
			  KEY_DOWN*/ 
   
  this->InitializeUI(m_Window);
  this->Render(1, 1, x, y);
  
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

    if (!currentWidget || !currentWidget->HandleInput(key, m_Form, m_Window))
      {
      // quit
      if ( key == 'q' )
	{
	break;
	}
      // if not end of page, next field otherwise next page
      else if ( key == KEY_DOWN || key == ctrl('n') )
	{
	int x,y;
	getmaxyx(m_Window, y, x);
	FIELD* cur = current_field(m_Form);
	int index = field_index(cur);
	if ( index == 3*m_Entries->size()-1 )
	  {
	  continue;
	  }
	if ( (index < 3*m_Entries->size()-1) && new_page(m_Fields[index+1]))
	  {
	  form_driver(m_Form, REQ_NEXT_PAGE);
	  }
	else
	  {
	  form_driver(m_Form, REQ_NEXT_FIELD);
	  }
	}
      // if not beginning of page, previous field, otherwise previous page
      else if ( key == KEY_UP || key == ctrl('p') )
	{
	int x,y;
	getmaxyx(m_Window, y, x);
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
      // generate and exit
      else if ( key == 'g' )
	{
	this->RunCMake(true);
	break;
	}
      // delete cache entry
      else if ( key == 'd' )
	{
	FIELD* cur = current_field(m_Form);
	int index = field_index(cur);

	// make the next or prev. current field after deletion
	FIELD* nextCur;
	if ( index == 2 )
	  {
	  }
	else if ( index == 3*m_Entries->size()-1 )
	  {
	  nextCur = m_Fields[index-5];
	  }
	else
	  {
	  nextCur = m_Fields[index+1];
	  }

	// Get the label widget
	cmCursesWidget* lbl = reinterpret_cast<cmCursesWidget*>(field_userptr(
	  m_Fields[index-2]));
	cmCacheManager::GetInstance()->RemoveCacheEntry(lbl->GetValue());

	std::string nextVal (reinterpret_cast<cmCursesWidget*>(field_userptr(nextCur))->GetValue());

	int x,y;
	getmaxyx(m_Window, y, x);
	this->InitializeUI(m_Window);
	this->Render(1, 1, x, y);

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

    touchwin(m_Window); 
    wrefresh(m_Window); 
    }
}
