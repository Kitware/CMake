#include "cmCursesPathWidget.h"

cmCursesPathWidget::cmCursesPathWidget(int width, int height, 
					   int left, int top) :
  cmCursesStringWidget(width, height, left, top)
{
  m_Type = cmCacheManager::PATH;
}

