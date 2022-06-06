/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmCursesCacheEntryComposite.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"
#include "cmStateTypes.h"

class cmake;
class cmCursesLongMessageForm;

/** \class cmCursesMainForm
 * \brief The main page of ccmake
 *
 * cmCursesMainForm is the main page of ccmake.
 */
class cmCursesMainForm : public cmCursesForm
{
public:
  cmCursesMainForm(std::vector<std::string> args, int initwidth);
  ~cmCursesMainForm() override;

  cmCursesMainForm(cmCursesMainForm const&) = delete;
  cmCursesMainForm& operator=(cmCursesMainForm const&) = delete;

  /**
   * Set the widgets which represent the cache entries.
   */
  void InitializeUI();

  /**
   * Handle user input.
   */
  void HandleInput() override;

  /**
   * Display form. Use a window of size width x height, starting
   * at top, left.
   */
  void Render(int left, int top, int width, int height) override;

  /**
   * Returns true if an entry with the given key is in the
   * list of current composites.
   */
  bool LookForCacheEntry(const std::string& key);

  enum
  {
    MIN_WIDTH = 65,
    MIN_HEIGHT = 6,
    IDEAL_WIDTH = 80,
    MAX_WIDTH = 512
  };

  /**
   * This method should normally be called only by the form.  The only
   * exception is during a resize. The optional argument specifies the
   * string to be displayed in the status bar.
   */
  void UpdateStatusBar() override { this->UpdateStatusBar(cm::nullopt); }
  void UpdateStatusBar(cm::optional<std::string> message);

  /**
   * Display current commands and their keys on the toolbar.  This
   * method should normally called only by the form.  The only
   * exception is during a resize. If the optional argument process is
   * specified and is either 1 (configure) or 2 (generate), then keys
   * will be displayed accordingly.
   */
  void PrintKeys(int process = 0);

  /**
   * During a CMake run, an error handle should add errors
   * to be displayed afterwards.
   */
  void AddError(const std::string& message, const char* title) override;

  /**
   * Used to do a configure. If argument is specified, it does only the check
   * and not configure.
   */
  int Configure(int noconfigure = 0);

  /**
   * Used to generate
   */
  int Generate();

  /**
   * Used by main program
   */
  int LoadCache(const char* dir);

  /**
   * Progress callback
   */
  void UpdateProgress(const std::string& msg, float prog);

protected:
  // Copy the cache values from the user interface to the actual
  // cache.
  void FillCacheManagerFromUI();
  // Fix formatting of values to a consistent form.
  void FixValue(cmStateEnums::CacheEntryType type, const std::string& in,
                std::string& out) const;
  // Re-post the existing fields. Used to toggle between
  // normal and advanced modes. Render() should be called
  // afterwards.
  void RePost();
  // Remove an entry from the interface and the cache.
  void RemoveEntry(const char* value);

  // Jump to the cache entry whose name matches the string.
  void JumpToCacheEntry(const char* str);

  // Clear and reset the output log and state
  void ResetOutputs();

  // Display the current progress and output
  void DisplayOutputs(std::string const& newOutput);

  // Copies of cache entries stored in the user interface
  std::vector<cmCursesCacheEntryComposite> Entries;

  // The form used to display logs during processing
  std::unique_ptr<cmCursesLongMessageForm> LogForm;
  // Output produced by the last pass
  std::vector<std::string> Outputs;
  // Did the last pass produced outputs of interest (errors, warnings, ...)
  bool HasNonStatusOutputs = false;
  // Last progress bar
  std::string LastProgress;

  // Command line arguments to be passed to cmake each time
  // it is run
  std::vector<std::string> Args;
  // Message displayed when user presses 'h'
  // It is: Welcome + info about current entry + common help
  std::vector<std::string> HelpMessage;

  // Common help
  static const char* s_ConstHelpMessage;

  // Fields displayed. Includes labels, new entry markers, entries
  std::vector<FIELD*> Fields;
  // Number of entries shown (depends on mode -normal or advanced-)
  size_t NumberOfVisibleEntries = 0;
  bool AdvancedMode = false;
  // Did the iteration converge (no new entries) ?
  bool OkToGenerate = false;
  // Number of pages displayed
  int NumberOfPages = 0;
  bool IsEmpty = false;
  std::unique_ptr<cmCursesCacheEntryComposite> EmptyCacheEntry;

  int InitialWidth;
  std::unique_ptr<cmake> CMakeInstance;

  std::string SearchString;
  std::string OldSearchString;
  bool SearchMode = false;
};
