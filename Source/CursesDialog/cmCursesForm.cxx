#include "cmCursesForm.h"

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
