#ifndef __cmCursesForm_h
#define __cmCursesForm_h

#include "cmCursesStandardIncludes.h"

class cmCursesForm
{
public:
  cmCursesForm();
  virtual ~cmCursesForm();
  
  // Description:
  // Handle user input.
  virtual void HandleInput() = 0;

  // Description:
  // Display form.
  virtual void Render(int left, int top, int width, int height) = 0;

protected:
  cmCursesForm(const cmCursesForm& from);
  void operator=(const cmCursesForm&);

  FORM* m_Form;
};

#endif // __cmCursesForm_h
