#include "cmCursesBoolWidget.h"

cmCursesBoolWidget::cmCursesBoolWidget(int width, int height, 
				       int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  m_Type = cmCacheManager::BOOL;
  set_field_fore(m_Field,  A_NORMAL);
  set_field_back(m_Field,  A_STANDOUT);
  field_opts_off(m_Field,  O_STATIC);
  this->SetValueAsBool(false);
}

bool cmCursesBoolWidget::HandleInput(int& key, FORM* form, WINDOW* w)
{
  // 10 == enter
  if (key == 10)
    {
    if (this->GetValueAsBool())
      {
      this->SetValueAsBool(false);
      }
    else
      {
      this->SetValueAsBool(true);
      }

    touchwin(w); 
    wrefresh(w); 
    return true;
    }
  else
    {
    return false;
    }
  
}

void cmCursesBoolWidget::SetValueAsBool(bool value)
{
  if (value)
    {
    this->SetValue("ON");
    }
  else
    { 
    this->SetValue("OFF");
    }
}

bool cmCursesBoolWidget::GetValueAsBool()
{
  if (m_Value == "ON")
    {
    return true;
    }
  else
    { 
    return false;
    }
}
