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

cmCursesMainForm::cmCursesMainForm(const char* whereSource, 
				   bool newCache) :
  m_WhereSource(whereSource)
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
  if ( width < 22 || height < 2 )
    {
    return;
    }

  std::cerr << "Rendering again." << std::endl;
  
  height -= 3;
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
  this->UpdateCurrentEntry();
  touchwin(m_Window); 
  refresh();
}

void cmCursesMainForm::UpdateCurrentEntry()
{
  FIELD* cur = current_field(m_Form);
  int index = field_index(cur);
  char* text = field_buffer(m_Fields[index-2], 0);

  int x,y;
  getmaxyx(m_Window, y, x);
  move(y-1,0);
  printw(text);

  char version[128];
  sprintf(version,"CMake Version %d.%d", cmMakefile::GetMajorVersion(),
	  cmMakefile::GetMinorVersion());
  int len = strlen(version);
  move(y-1, x-len);
  printw(version);

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
  args.push_back("cmake");
  std::string arg;
  arg = m_WhereSource;
  args.push_back(arg);
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
    this->UpdateCurrentEntry();
    int key = getch();

    currentField = current_field(m_Form);
    currentWidget = reinterpret_cast<cmCursesWidget*>(field_userptr(
      currentField));

    if (!currentWidget || !currentWidget->HandleInput(key, m_Form, m_Window))
      {
      if ( key == 'q' )
	{
	break;
	}
      else if ( key == KEY_DOWN )
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
      else if ( key == KEY_UP )
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
      else if ( key == KEY_NPAGE )
	{
	form_driver(m_Form, REQ_NEXT_PAGE);
	}
      else if ( key == KEY_PPAGE )
	{
	form_driver(m_Form, REQ_PREV_PAGE);
	}
      else if ( key == 'c' )
	{
	this->RunCMake(false);
	}
      else if ( key == 'o' )
	{
	this->RunCMake(true);
	break;
	}
      }

    touchwin(m_Window); 
    wrefresh(m_Window); 
    }
}
