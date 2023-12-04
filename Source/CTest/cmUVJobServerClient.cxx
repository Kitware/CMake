/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUVJobServerClient.h"

#include <cassert>
#include <utility>

#ifndef _WIN32
#  include <cstdio>
#  include <string>
#  include <vector>

#  include <fcntl.h>
#  include <unistd.h>
#endif

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>

#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"

class cmUVJobServerClient::Impl
{
public:
  uv_loop_t& Loop;

  cm::uv_idle_ptr ImplicitToken;
  std::function<void()> OnToken;
  std::function<void(int)> OnDisconnect;

  // The number of tokens held by this client.
  unsigned int HeldTokens = 0;

  // The number of tokens we need to receive from the job server.
  unsigned int NeedTokens = 0;

  Impl(uv_loop_t& loop);
  virtual ~Impl();

  virtual void SendToken() = 0;
  virtual void StartReceivingTokens() = 0;
  virtual void StopReceivingTokens() = 0;

  void RequestToken();
  void ReleaseToken();
  void RequestExplicitToken();
  void DecrementNeedTokens();
  void HoldToken();
  void RequestImplicitToken();
  void ReleaseImplicitToken();
  void ReceivedToken();
  void Disconnected(int status);
};

cmUVJobServerClient::Impl::Impl(uv_loop_t& loop)
  : Loop(loop)
{
  this->ImplicitToken.init(this->Loop, this);
}

cmUVJobServerClient::Impl::~Impl() = default;

void cmUVJobServerClient::Impl::RequestToken()
{
  if (this->HeldTokens == 0 && !uv_is_active(this->ImplicitToken)) {
    this->RequestImplicitToken();
  } else {
    this->RequestExplicitToken();
  }
}

void cmUVJobServerClient::Impl::ReleaseToken()
{
  assert(this->HeldTokens > 0);
  --this->HeldTokens;
  if (this->HeldTokens == 0) {
    // This was the token implicitly owned by our process.
    this->ReleaseImplicitToken();
  } else {
    // This was a token we received from the job server.  Send it back.
    this->SendToken();
  }
}

void cmUVJobServerClient::Impl::RequestExplicitToken()
{
  ++this->NeedTokens;
  this->StartReceivingTokens();
}

void cmUVJobServerClient::Impl::DecrementNeedTokens()
{
  assert(this->NeedTokens > 0);
  --this->NeedTokens;
  if (this->NeedTokens == 0) {
    this->StopReceivingTokens();
  }
}

void cmUVJobServerClient::Impl::HoldToken()
{
  ++this->HeldTokens;
  if (this->OnToken) {
    this->OnToken();
  } else {
    this->ReleaseToken();
  }
}

void cmUVJobServerClient::Impl::RequestImplicitToken()
{
  assert(this->HeldTokens == 0);
  this->ImplicitToken.start([](uv_idle_t* handle) {
    uv_idle_stop(handle);
    auto* self = static_cast<Impl*>(handle->data);
    self->HoldToken();
  });
}

void cmUVJobServerClient::Impl::ReleaseImplicitToken()
{
  assert(this->HeldTokens == 0);
  // Use the implicit token in place of receiving one from the job server.
  if (this->NeedTokens > 0) {
    this->DecrementNeedTokens();
    this->RequestImplicitToken();
  }
}

void cmUVJobServerClient::Impl::ReceivedToken()
{
  this->DecrementNeedTokens();
  this->HoldToken();
}

void cmUVJobServerClient::Impl::Disconnected(int status)
{
  if (this->OnDisconnect) {
    this->OnDisconnect(status);
  }
}

//---------------------------------------------------------------------------
// Implementation on POSIX platforms.
// https://www.gnu.org/software/make/manual/html_node/POSIX-Jobserver.html

#ifndef _WIN32
namespace {
class ImplPosix : public cmUVJobServerClient::Impl
{
public:
  enum class Connection
  {
    None,
    FDs,
    FIFO,
  };
  Connection Conn = Connection::None;

  cm::uv_pipe_ptr ConnRead;
  cm::uv_pipe_ptr ConnWrite;
  cm::uv_pipe_ptr ConnFIFO;

  std::shared_ptr<std::function<void(int)>> OnWrite;

  void Connect();
  void ConnectFDs(int rfd, int wfd);
  void ConnectFIFO(const char* path);
  void Disconnect(int status);

  cm::uv_pipe_ptr OpenFD(int fd);

  uv_stream_t* GetWriter() const;
  uv_stream_t* GetReader() const;

  static void OnAllocateCB(uv_handle_t* handle, size_t suggested_size,
                           uv_buf_t* buf);
  static void OnReadCB(uv_stream_t* stream, ssize_t nread,
                       const uv_buf_t* buf);

  void OnAllocate(size_t suggested_size, uv_buf_t* buf);
  void OnRead(ssize_t nread, const uv_buf_t* buf);

  char ReadBuf = '.';

  bool ReceivingTokens = false;

  bool IsConnected() const;

  void SendToken() override;
  void StartReceivingTokens() override;
  void StopReceivingTokens() override;

  ImplPosix(uv_loop_t& loop);
  ~ImplPosix() override;
};

ImplPosix::ImplPosix(uv_loop_t& loop)
  : Impl(loop)
  , OnWrite(std::make_shared<std::function<void(int)>>([this](int status) {
    if (status != 0) {
      this->Disconnect(status);
    }
  }))
{
  this->Connect();
}

ImplPosix::~ImplPosix()
{
  this->Disconnect(0);
}

void ImplPosix::Connect()
{
  // --jobserver-auth= for gnu make versions >= 4.2
  // --jobserver-fds= for gnu make versions < 4.2
  // -J for bsd make
  static const std::vector<cm::string_view> prefixes = {
    "--jobserver-auth=", "--jobserver-fds=", "-J"
  };

  cm::optional<std::string> makeflags = cmSystemTools::GetEnvVar("MAKEFLAGS");
  if (!makeflags) {
    return;
  }

  // Look for the *last* occurrence of jobserver flags.
  cm::optional<std::string> auth;
  std::vector<std::string> args;
  cmSystemTools::ParseUnixCommandLine(makeflags->c_str(), args);
  for (cm::string_view arg : cmReverseRange(args)) {
    for (cm::string_view prefix : prefixes) {
      if (cmHasPrefix(arg, prefix)) {
        auth = cmTrimWhitespace(arg.substr(prefix.length()));
        break;
      }
    }
    if (auth) {
      break;
    }
  }

  if (!auth) {
    return;
  }

  // fifo:PATH
  if (cmHasLiteralPrefix(*auth, "fifo:")) {
    ConnectFIFO(auth->substr(cmStrLen("fifo:")).c_str());
    return;
  }

  // reader,writer
  int reader;
  int writer;
  if (std::sscanf(auth->c_str(), "%d,%d", &reader, &writer) == 2) {
    ConnectFDs(reader, writer);
  }
}

cm::uv_pipe_ptr ImplPosix::OpenFD(int fd)
{
  // Create a CLOEXEC duplicate so `uv_pipe_ptr` can close it
  // without closing the original file descriptor, which our
  // child processes might want to use too.
  cm::uv_pipe_ptr p;
  int fd_dup = dup(fd);
  if (fd_dup < 0) {
    return p;
  }
  if (fcntl(fd_dup, F_SETFD, FD_CLOEXEC) == -1) {
    close(fd_dup);
    return p;
  }
  p.init(this->Loop, 0, this);
  if (uv_pipe_open(p, fd_dup) < 0) {
    close(fd_dup);
  }
  return p;
}

void ImplPosix::ConnectFDs(int rfd, int wfd)
{
  cm::uv_pipe_ptr connRead = this->OpenFD(rfd);
  cm::uv_pipe_ptr connWrite = this->OpenFD(wfd);

  // Verify that the read end is readable and the write end is writable.
  if (!connRead || !uv_is_readable(connRead) || //
      !connWrite || !uv_is_writable(connWrite)) {
    return;
  }

  this->ConnRead = std::move(connRead);
  this->ConnWrite = std::move(connWrite);
  this->Conn = Connection::FDs;
}

void ImplPosix::ConnectFIFO(const char* path)
{
  int fd = open(path, O_RDWR);
  if (fd < 0) {
    return;
  }

  cm::uv_pipe_ptr connFIFO;
  connFIFO.init(this->Loop, 0, this);
  if (uv_pipe_open(connFIFO, fd) != 0) {
    close(fd);
    return;
  }

  // Verify that the fifo is both readable and writable.
  if (!connFIFO || !uv_is_readable(connFIFO) || !uv_is_writable(connFIFO)) {
    return;
  }

  this->ConnFIFO = std::move(connFIFO);
  this->Conn = Connection::FIFO;
}

void ImplPosix::Disconnect(int status)
{
  if (this->Conn == Connection::None) {
    return;
  }

  this->StopReceivingTokens();

  switch (this->Conn) {
    case Connection::FDs:
      this->ConnRead.reset();
      this->ConnWrite.reset();
      break;
    case Connection::FIFO:
      this->ConnFIFO.reset();
      break;
    default:
      break;
  }

  this->Conn = Connection::None;
  if (status != 0) {
    this->Disconnected(status);
  }
}

uv_stream_t* ImplPosix::GetWriter() const
{
  switch (this->Conn) {
    case Connection::FDs:
      return this->ConnWrite;
    case Connection::FIFO:
      return this->ConnFIFO;
    default:
      return nullptr;
  }
}

uv_stream_t* ImplPosix::GetReader() const
{
  switch (this->Conn) {
    case Connection::FDs:
      return this->ConnRead;
    case Connection::FIFO:
      return this->ConnFIFO;
    default:
      return nullptr;
  }
}

void ImplPosix::OnAllocateCB(uv_handle_t* handle, size_t suggested_size,
                             uv_buf_t* buf)
{
  auto* self = static_cast<ImplPosix*>(handle->data);
  self->OnAllocate(suggested_size, buf);
}

void ImplPosix::OnReadCB(uv_stream_t* stream, ssize_t nread,
                         const uv_buf_t* buf)
{
  auto* self = static_cast<ImplPosix*>(stream->data);
  self->OnRead(nread, buf);
}

void ImplPosix::OnAllocate(size_t /*suggested_size*/, uv_buf_t* buf)
{
  *buf = uv_buf_init(&this->ReadBuf, 1);
}

void ImplPosix::OnRead(ssize_t nread, const uv_buf_t* /*buf*/)
{
  if (nread == 0) {
    return;
  }

  if (nread < 0) {
    auto status = static_cast<int>(nread);
    this->Disconnect(status);
    return;
  }

  assert(nread == 1);
  this->ReceivedToken();
}

bool ImplPosix::IsConnected() const
{
  return this->Conn != Connection::None;
}

void ImplPosix::SendToken()
{
  if (this->Conn == Connection::None) {
    return;
  }

  static char token = '.';

  uv_buf_t const buf = uv_buf_init(&token, sizeof(token));
  int status = cm::uv_write(this->GetWriter(), &buf, 1, this->OnWrite);
  if (status != 0) {
    this->Disconnect(status);
  }
}

void ImplPosix::StartReceivingTokens()
{
  if (this->Conn == Connection::None) {
    return;
  }
  if (this->ReceivingTokens) {
    return;
  }

  int status = uv_read_start(this->GetReader(), &ImplPosix::OnAllocateCB,
                             &ImplPosix::OnReadCB);
  if (status != 0) {
    this->Disconnect(status);
    return;
  }

  this->ReceivingTokens = true;
}

void ImplPosix::StopReceivingTokens()
{
  if (this->Conn == Connection::None) {
    return;
  }
  if (!this->ReceivingTokens) {
    return;
  }

  this->ReceivingTokens = false;
  uv_read_stop(this->GetReader());
}
}
#endif

//---------------------------------------------------------------------------
// Implementation of public interface.

cmUVJobServerClient::cmUVJobServerClient(std::unique_ptr<Impl> impl)
  : Impl_(std::move(impl))
{
}

cmUVJobServerClient::~cmUVJobServerClient() = default;

cmUVJobServerClient::cmUVJobServerClient(cmUVJobServerClient&&) noexcept =
  default;
cmUVJobServerClient& cmUVJobServerClient::operator=(
  cmUVJobServerClient&&) noexcept = default;

void cmUVJobServerClient::RequestToken()
{
  this->Impl_->RequestToken();
}

void cmUVJobServerClient::ReleaseToken()
{
  this->Impl_->ReleaseToken();
}

int cmUVJobServerClient::GetHeldTokens() const
{
  return this->Impl_->HeldTokens;
}

int cmUVJobServerClient::GetNeedTokens() const
{
  return this->Impl_->NeedTokens;
}

cm::optional<cmUVJobServerClient> cmUVJobServerClient::Connect(
  uv_loop_t& loop, std::function<void()> onToken,
  std::function<void(int)> onDisconnect)
{
#if defined(_WIN32)
  // FIXME: Windows job server client not yet implemented.
  static_cast<void>(loop);
  static_cast<void>(onToken);
  static_cast<void>(onDisconnect);
#else
  auto impl = cm::make_unique<ImplPosix>(loop);
  if (impl && impl->IsConnected()) {
    impl->OnToken = std::move(onToken);
    impl->OnDisconnect = std::move(onDisconnect);
    return cmUVJobServerClient(std::move(impl));
  }
#endif
  return cm::nullopt;
}
