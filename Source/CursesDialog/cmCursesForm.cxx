#include "cmCursesForm.h"

std::ofstream cmCursesForm::DebugFile;
bool cmCursesForm::Debug = false;

cmCursesForm::cmCursesForm()
{
  m_Form = 0;
}

cmCursesForm::~cmCursesForm()
{
  if (m_Form)
    {
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }
}

void cmCursesForm::DebugStart()
{
  cmCursesForm::Debug = true;
  cmCursesForm::DebugFile.open("ccmakelog.txt");
}

void cmCursesForm::DebugEnd()
{
  if (!cmCursesForm::Debug)
    {
    return;
    }

  cmCursesForm::Debug = false;
  cmCursesForm::DebugFile.close();
}

void cmCursesForm::LogMessage(const char* msg)
{
  if (!cmCursesForm::Debug)
    {
    return;
    }

  cmCursesForm::DebugFile << msg << std::endl;
}
