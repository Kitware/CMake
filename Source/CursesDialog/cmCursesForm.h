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

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar() = 0;

  // During a CMake run, an error handle should add errors
  // to be displayed afterwards.
  virtual void AddError(const char* message, const char* title) {};

  static cmCursesForm* CurrentForm;

protected:
  cmCursesForm(const cmCursesForm& from);
  void operator=(const cmCursesForm&);

  FORM* m_Form;
};

#endif // __cmCursesForm_h
