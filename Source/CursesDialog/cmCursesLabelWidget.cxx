#include "cmCursesLabelWidget.h"

cmCursesLabelWidget::cmCursesLabelWidget(int width, int height, 
					 int left, int top,
					 const std::string& name) :
  cmCursesWidget(width, height, left, top)
{
  field_opts_off(m_Field,  O_EDIT);
  field_opts_off(m_Field,  O_ACTIVE);
  field_opts_off(m_Field,  O_STATIC);
  this->SetValue(name.c_str());
}

cmCursesLabelWidget::~cmCursesLabelWidget()
{
}

bool cmCursesLabelWidget::HandleInput(int& key, cmCursesMainForm*, WINDOW* )
{
  // Static text. No input is handled here.
  return false;
}
