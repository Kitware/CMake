#include "cmCursesDummyWidget.h"

cmCursesDummyWidget::cmCursesDummyWidget(int width, int height, 
					   int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  m_Type = cmCacheManager::INTERNAL;
}


bool cmCursesDummyWidget::HandleInput(int& key, FORM* form, WINDOW* w)
{
  return false;
}

