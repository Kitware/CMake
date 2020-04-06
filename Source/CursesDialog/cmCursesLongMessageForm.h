/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCursesLongMessageForm_h
#define cmCursesLongMessageForm_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesLongMessageForm : public cmCursesForm
{
public:
  enum class ScrollBehavior
  {
    NoScroll,
    ScrollDown
  };

  cmCursesLongMessageForm(std::vector<std::string> const& messages,
                          const char* title, ScrollBehavior scrollBehavior);
  ~cmCursesLongMessageForm() override;

  cmCursesLongMessageForm(cmCursesLongMessageForm const&) = delete;
  cmCursesLongMessageForm& operator=(cmCursesLongMessageForm const&) = delete;

  void UpdateContent(std::string const& output, std::string const& title);

  // Description:
  // Handle user input.
  void HandleInput() override;

  // Description:
  // Display form. Use a window of size width x height, starting
  // at top, left.
  void Render(int left, int top, int width, int height) override;

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void PrintKeys();

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void UpdateStatusBar() override;

protected:
  static constexpr int MAX_CONTENT_SIZE = 60000;

  void DrawMessage(const char* msg) const;

  std::string Messages;
  std::string Title;
  ScrollBehavior Scrolling;

  FIELD* Fields[2];
};

#endif // cmCursesLongMessageForm_h
