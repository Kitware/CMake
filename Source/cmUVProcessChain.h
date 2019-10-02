/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmUVProcessChain_h
#define cmUVProcessChain_h

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cm_uv.h"

class cmUVProcessChain;

class cmUVProcessChainBuilder
{
public:
  enum Stream
  {
    Stream_INPUT = 0,
    Stream_OUTPUT = 1,
    Stream_ERROR = 2,
  };

  cmUVProcessChainBuilder();

  cmUVProcessChainBuilder& AddCommand(
    const std::vector<std::string>& arguments);
  cmUVProcessChainBuilder& SetNoStream(Stream stdio);
  cmUVProcessChainBuilder& SetBuiltinStream(Stream stdio);
  cmUVProcessChainBuilder& SetExternalStream(Stream stdio, int fd);

  cmUVProcessChain Start() const;

private:
  enum StdioType
  {
    None,
    Builtin,
    External,
  };

  friend class cmUVProcessChain;

  struct StdioConfiguration
  {
    StdioType Type;
    int FileDescriptor;
  };

  struct ProcessConfiguration
  {
    std::vector<std::string> Arguments;
  };

  std::array<StdioConfiguration, 3> Stdio;
  std::vector<ProcessConfiguration> Processes;
};

class cmUVProcessChain
{
public:
  struct Status
  {
    int64_t ExitStatus;
    int TermSignal;
  };

  cmUVProcessChain(const cmUVProcessChain& other) = delete;
  cmUVProcessChain(cmUVProcessChain&& other) noexcept;

  ~cmUVProcessChain();

  cmUVProcessChain& operator=(const cmUVProcessChain& other) = delete;
  cmUVProcessChain& operator=(cmUVProcessChain&& other) noexcept;

  uv_loop_t& GetLoop();

  // FIXME: Add stdin support
  std::istream* OutputStream();
  std::istream* ErrorStream();

  bool Valid() const;
  bool Wait(int64_t milliseconds = -1);
  std::vector<const Status*> GetStatus() const;
  const Status* GetStatus(std::size_t index) const;

private:
  friend class cmUVProcessChainBuilder;

  cmUVProcessChain();

  struct InternalData;
  std::unique_ptr<InternalData> Data;
};

#endif
