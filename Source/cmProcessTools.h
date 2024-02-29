/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstring>
#include <iosfwd>
#include <string>

#include "cmProcessOutput.h"

/** \class cmProcessTools
 * \brief Helper classes for process output parsing
 *
 */
class cmProcessTools
{
public:
  using Encoding = cmProcessOutput::Encoding;
  /** Abstract interface for process output parsers.  */
  class OutputParser
  {
  public:
    /** Process the given output data from a tool.  Processing may be
        done incrementally.  Returns true if the parser is interested
        in any more data and false if it is done.  */
    bool Process(const char* data, int length)
    {
      return this->ProcessChunk(data, length);
    }
    bool Process(const char* data)
    {
      return this->Process(data, static_cast<int>(strlen(data)));
    }

    virtual ~OutputParser() = default;

  protected:
    /** Implement in a subclass to process a chunk of data.  It should
        return true only if it is interested in more data.  */
    virtual bool ProcessChunk(const char* data, int length) = 0;
  };

  /** Process output parser that extracts one line at a time.  */
  class LineParser : public OutputParser
  {
  public:
    /** Construct with line separation character and choose whether to
        ignore carriage returns.  */
    LineParser(char sep = '\n', bool ignoreCR = true);

    /** Configure logging of lines as they are extracted.  */
    void SetLog(std::ostream* log, const char* prefix);

  protected:
    std::ostream* Log = nullptr;
    const char* Prefix = nullptr;
    std::string Line;
    char Separator;
    char LineEnd = '\0';
    bool IgnoreCR;
    bool ProcessChunk(const char* data, int length) override;

    /** Implement in a subclass to process one line of input.  It
        should return true only if it is interested in more data.  */
    virtual bool ProcessLine() = 0;
  };

  /** Trivial line handler for simple logging.  */
  class OutputLogger : public LineParser
  {
  public:
    OutputLogger(std::ostream& log, const char* prefix = nullptr)
    {
      this->SetLog(&log, prefix);
    }

  private:
    bool ProcessLine() override { return true; }
  };

  /** Run a process and send output to given parsers.  */
  static void RunProcess(struct cmsysProcess_s* cp, OutputParser* out,
                         OutputParser* err = nullptr,
                         Encoding encoding = cmProcessOutput::Auto);
};
