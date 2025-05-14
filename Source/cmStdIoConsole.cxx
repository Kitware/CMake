/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmStdIoConsole.h"

#ifdef _WIN32
#  include <cstddef>
#  include <cstdlib>
#  include <ios>
#  include <streambuf>
#  include <utility>
#  include <vector>

#  include <cm/memory>

#  include <windows.h>

#  include <fcntl.h> // for _O_BINARY
#  include <io.h>    // for _setmode

#  include "cm_utf8.h"

#  include "cmStdIoStream.h"
#endif

namespace cm {
namespace StdIo {

namespace {

#ifdef _WIN32
// Base class for a streambuf that reads or writes a Windows Console.
class ConsoleBuf : public std::streambuf
{
public:
  ConsoleBuf(HANDLE console)
    : console_(console)
  {
  }

  ~ConsoleBuf() throw() override {}

protected:
  HANDLE console_ = nullptr;
};

// A streambuf that reads from a Windows Console using wide-character
// encoding to avoid conversion through the console output code page.
class ConsoleBufRead : public ConsoleBuf
{
public:
  ConsoleBufRead(HANDLE console, DWORD consoleMode)
    : ConsoleBuf(console)
    , ConsoleMode_(consoleMode)
  {
  }

  ~ConsoleBufRead() throw() override {}

protected:
  // Called to read an input character when the input buffer may be empty.
  int_type underflow() override
  {
    // If the input buffer is not empty, return the next input character.
    if (this->gptr() < this->egptr()) {
      return traits_type::to_int_type(*this->gptr());
    }

    // The input buffer is empty.  Read more input from the console.
    static constexpr std::size_t kBufSize = 4096;
    this->TmpW_.resize(kBufSize);
    DWORD wlen = 0;
    if (!ReadConsoleW(this->console_, this->TmpW_.data(),
                      DWORD(this->TmpW_.size()), &wlen, nullptr)) {
      // Failure.  Nothing was read.
      return traits_type::eof();
    }

    // Emulate ReadFile behavior when the console is in "cooked mode".
    // Treat a leading Ctrl+Z as EOF.
    static constexpr char ctrl_z = 26; // Ctrl+Z is Ctrl + 26th letter.
    if ((this->ConsoleMode_ & ENABLE_LINE_INPUT) &&
        (wlen > 0 && this->TmpW_.front() == ctrl_z)) {
      wlen = 0;
    }

    // Convert the wide-character encoding from the console to our
    // internal UTF-8 narrow encoding.
    if (int nlen =
          WideCharToMultiByte(CP_UTF8, 0, this->TmpW_.data(), int(wlen),
                              nullptr, 0, nullptr, nullptr)) {
      this->Buf_.resize(nlen);
      if (WideCharToMultiByte(CP_UTF8, 0, this->TmpW_.data(), int(wlen),
                              this->Buf_.data(), int(nlen), nullptr,
                              nullptr)) {
        // The converted content is now in the input buffer.
        this->setg_();

        // Success.  Return the next input character.
        return traits_type::to_int_type(*this->gptr());
      }
    }

    // Failure.  Nothing was read.
    return traits_type::eof();
  }

private:
  DWORD ConsoleMode_ = 0;
  std::vector<char> Buf_;
  std::vector<wchar_t> TmpW_;

  // Set input buffer pointers.
  void setg_()
  {
    this->setg(this->Buf_.data(), this->Buf_.data(),
               this->Buf_.data() + this->Buf_.size());
  }
};

// A streambuf that writes to a Windows Console using wide-character
// encoding to avoid conversion through the console output code page.
class ConsoleBufWrite : public ConsoleBuf
{
public:
  ConsoleBufWrite(HANDLE console)
    : ConsoleBuf(console)
  {
    this->setp_();
  }

  ~ConsoleBufWrite() throw() override { sync(); }

protected:
  // Called to sync input and output buffers with the underlying device.
  int sync() override
  {
    // Flush buffered output, if any.
    if (this->pptr() != this->pbase()) {
      // Use overflow() to flush the entire output buffer.
      // It returns eof on failure.
      if (traits_type::eq_int_type(this->overflow(), traits_type::eof())) {
        return -1;
      }
    }
    return 0;
  }

  // Called to flush at least some content from the output buffer.
  int_type overflow(int_type ch = traits_type::eof()) override
  {
    std::size_t nlen;     // Number of chars to emit.
    std::size_t rlen = 0; // Number of chars to roll over.
    if (traits_type::eq_int_type(ch, traits_type::eof())) {
      // Our caller wants to flush the entire buffer.  If there is a
      // trailing partial codepoint, it's the caller's fault.
      nlen = this->pptr() - this->pbase();

      // If the buffer is empty, trivially succeed.
      if (nlen == 0) {
        return traits_type::not_eof(ch);
      }
    } else {
      // Our caller had no room for this character in the buffer.
      // However, setp_() reserved one byte for us to store it.
      *this->pptr() = traits_type::to_char_type(ch);
      this->pbump(1);

      // Flush all complete codepoints, of which we expect at least one.
      // If there is a trailing partial codepoint, roll over those chars.
      char const* p = this->pptr_();
      nlen = p - this->pbase();
      rlen = this->pptr() - p;
    }

    // Fail unless we emit at least one (wide) character.
    int_type result = traits_type::eof();

    // Convert our internal UTF-8 narrow encoding to wide-character
    // encoding to write to the console.
    if (int wlen = MultiByteToWideChar(CP_UTF8, 0, this->pbase(), int(nlen),
                                       nullptr, 0)) {
      this->TmpW_.resize(wlen);
      if (MultiByteToWideChar(CP_UTF8, 0, this->pbase(), int(nlen),
                              this->TmpW_.data(), int(wlen)) &&
          WriteConsoleW(this->console_, this->TmpW_.data(), wlen, nullptr,
                        nullptr)) {
        result = traits_type::not_eof(ch);
      }
    }

    // Remove emitted contents from the buffer.
    this->Buf_.erase(this->Buf_.begin(), this->Buf_.begin() + nlen);

    // Re-initialize the output buffer.
    this->setp_();

    // Move the put-pointer past the rollover content.
    this->pbump(rlen);

    return result;
  }

private:
  std::vector<char> Buf_;
  std::vector<wchar_t> TmpW_;

  // Initialize the output buffer and set its put-pointer.
  void setp_()
  {
    // Allocate the output buffer.
    static constexpr std::size_t kBufSize = 4096;
    this->Buf_.resize(kBufSize);

    // Reserve one byte for the overflow() character.
    this->setp(this->Buf_.data(), this->Buf_.data() + this->Buf_.size() - 1);
  }

  // Return pptr() adjusted backward past a partial codepoint.
  char const* pptr_() const
  {
    char const* p = this->pptr();
    while (p != this->pbase()) {
      --p;
      switch (cm_utf8_ones[static_cast<unsigned char>(*p)]) {
        case 0: // 0xxx xxxx: starts codepoint of size 1
          return p + 1;
        case 1: // 10xx xxxx: continues a codepoint
          continue;
        case 2: // 110x xxxx: starts codepoint of size 2
          return ((p + 2) <= this->pptr()) ? (p + 2) : p;
        case 3: // 1110 xxxx: starts codepoint of size 3
          return ((p + 3) <= this->pptr()) ? (p + 3) : p;
        case 4: // 1111 0xxx: starts codepoint of size 4
          return ((p + 4) <= this->pptr()) ? (p + 4) : p;
        default: // invalid byte
          // Roll over the invalid byte.
          // The next overflow() will fail to convert it.
          return p;
      }
    }
    // No complete codepoint found.  This overflow() will fail.
    return p;
  }
};

#endif

} // anonymous namespace

#ifdef _WIN32
class Console::Impl
{
protected:
  class RAII
  {
    std::ios* IOS_ = nullptr;
    int FD_ = -1;
    std::unique_ptr<ConsoleBuf> ConsoleBuf_;
    std::streambuf* OldStreamBuf_ = nullptr;
    int OldMode_ = 0;

    RAII(Stream& s);
    void Init();

  public:
    RAII(IStream& is);
    RAII(OStream& os);
    ~RAII();
  };
  RAII In_;
  RAII Out_;
  RAII Err_;

public:
  Impl();
  ~Impl();
};

Console::Impl::RAII::RAII(Stream& s)
  : IOS_(&s.IOS())
  , FD_(s.FD())
{
}

Console::Impl::RAII::RAII(IStream& is)
  : RAII(static_cast<Stream&>(is))
{
  DWORD mode;
  if (is.Console() && GetConsoleMode(is.Console(), &mode) &&
      GetConsoleCP() != CP_UTF8) {
    // The input stream reads from a console whose input code page is not
    // UTF-8.  Use a ConsoleBufRead to read wide-character encoding.
    this->ConsoleBuf_ = cm::make_unique<ConsoleBufRead>(is.Console(), mode);
  }
  this->Init();
}

Console::Impl::RAII::RAII(OStream& os)
  : RAII(static_cast<Stream&>(os))
{
  DWORD mode;
  if (os.Console() && GetConsoleMode(os.Console(), &mode) &&
      GetConsoleOutputCP() != CP_UTF8) {
    // The output stream writes to a console whose output code page is not
    // UTF-8.  Use a ConsoleBufWrite to write wide-character encoding.
    this->ConsoleBuf_ = cm::make_unique<ConsoleBufWrite>(os.Console());
  }
  this->Init();
}

void Console::Impl::RAII::Init()
{
  if (this->ConsoleBuf_) {
    this->OldStreamBuf_ = this->IOS_->rdbuf(this->ConsoleBuf_.get());
  } else if (this->FD_ >= 0) {
    // The stream reads/writes a pipe, a file, or a console whose code
    // page is UTF-8.  Read/write UTF-8 using the default streambuf,
    // but disable newline conversion to match ConsoleBuf behavior.
    this->OldMode_ = _setmode(this->FD_, _O_BINARY);
  }
}

Console::Impl::RAII::~RAII()
{
  if (this->ConsoleBuf_) {
    this->IOS_->rdbuf(this->OldStreamBuf_);
    this->OldStreamBuf_ = nullptr;
    this->ConsoleBuf_.reset();
  } else if (this->FD_ >= 0) {
    this->IOS_->rdbuf()->pubsync();
    _setmode(this->FD_, this->OldMode_);
    this->OldMode_ = 0;
  }
  this->FD_ = -1;
  this->IOS_ = nullptr;
}

Console::Impl::Impl()
  : In_(In())
  , Out_(Out())
  , Err_(Err())
{
}

Console::Impl::~Impl() = default;

Console::Console()
  : Impl_(cm::make_unique<Impl>())
{
}
#else
Console::Console() = default;
#endif

Console::~Console() = default;

Console::Console(Console&&) noexcept = default;
Console& Console::operator=(Console&&) noexcept = default;

}
}
