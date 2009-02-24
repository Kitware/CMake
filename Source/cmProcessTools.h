/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmProcessTools_h
#define cmProcessTools_h

#include "cmStandardIncludes.h"

/** \class cmProcessTools
 * \brief Helper classes for process output parsing
 *
 */
class cmProcessTools
{
public:
  /** Abstract interface for process output parsers.  */
  class OutputParser
  {
  public:
    /** Process the given output data from a tool.  Processing may be
        done incrementally.  Returns true if the parser is interested
        in any more data and false if it is done.  */
    bool Process(const char* data, int length)
      { return this->ProcessChunk(data, length); }
  protected:
    /** Implement in a subclass to process a chunk of data.  It should
        return true only if it is interested in more data.  */
    virtual bool ProcessChunk(const char* data, int length) = 0;
  };

  /** Process output parser that extracts one line at a time.  */
  class LineParser: public OutputParser
  {
  public:
    /** Construct with line separation character and choose whether to
        ignore carriage returns.  */
    LineParser(char sep = '\n', bool ignoreCR = true);

    /** Configure logging of lines as they are extracted.  */
    void SetLog(std::ostream* log, const char* prefix);
  protected:
    char Separator;
    bool IgnoreCR;
    std::ostream* Log;
    const char* Prefix;
    std::string Line;
    virtual bool ProcessChunk(const char* data, int length);

    /** Implement in a subclass to process one line of input.  It
        should return true only if it is interested in more data.  */
    virtual bool ProcessLine() = 0;
  };

  /** Trivial line handler for simple logging.  */
  class OutputLogger: public LineParser
  {
  public:
    OutputLogger(std::ostream& log, const char* prefix = 0)
      { this->SetLog(&log, prefix); }
  private:
    virtual bool ProcessLine() { return true; }
  };

  /** Run a process and send output to given parsers.  */
  static void RunProcess(struct cmsysProcess_s* cp,
                         OutputParser* out, OutputParser* err = 0);
};

#endif
