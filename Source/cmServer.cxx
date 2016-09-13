/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>
  Copyright 2016 Tobias Hunger <tobias.hunger@qt.io>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmServer.h"

#include "cmServerProtocol.h"
#include "cmVersionMacros.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#endif

const char kTYPE_KEY[] = "type";
const char kCOOKIE_KEY[] = "cookie";
const char REPLY_TO_KEY[] = "inReplyTo";
const char ERROR_MESSAGE_KEY[] = "errorMessage";

const char ERROR_TYPE[] = "error";
const char REPLY_TYPE[] = "reply";
const char PROGRESS_TYPE[] = "progress";

const char START_MAGIC[] = "[== CMake Server ==[";
const char END_MAGIC[] = "]== CMake Server ==]";

typedef struct
{
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
  (void)handle;
  *buf = uv_buf_init(static_cast<char*>(malloc(suggested_size)),
                     static_cast<unsigned int>(suggested_size));
}

void free_write_req(uv_write_t* req)
{
  write_req_t* wr = reinterpret_cast<write_req_t*>(req);
  free(wr->buf.base);
  free(wr);
}

void on_stdout_write(uv_write_t* req, int status)
{
  (void)status;
  auto server = reinterpret_cast<cmServer*>(req->data);
  free_write_req(req);
  server->PopOne();
}

void write_data(uv_stream_t* dest, std::string content, uv_write_cb cb)
{
  write_req_t* req = static_cast<write_req_t*>(malloc(sizeof(write_req_t)));
  req->req.data = dest->data;
  req->buf = uv_buf_init(static_cast<char*>(malloc(content.size())),
                         static_cast<unsigned int>(content.size()));
  memcpy(req->buf.base, content.c_str(), content.size());
  uv_write(reinterpret_cast<uv_write_t*>(req), static_cast<uv_stream_t*>(dest),
           &req->buf, 1, cb);
}

void read_stdin(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
  if (nread > 0) {
    auto server = reinterpret_cast<cmServer*>(stream->data);
    std::string result = std::string(buf->base, buf->base + nread);
    server->handleData(result);
  }

  if (buf->base)
    free(buf->base);
}

cmServer::cmServer()
{
  // Register supported protocols:
  this->RegisterProtocol(new cmServerProtocol1_0);
}

cmServer::~cmServer()
{
  if (!this->Protocol) // Daemon was never fully started!
    return;

  uv_close(reinterpret_cast<uv_handle_t*>(this->InputStream), NULL);
  uv_close(reinterpret_cast<uv_handle_t*>(this->OutputStream), NULL);
  uv_loop_close(this->Loop);

  for (cmServerProtocol* p : this->SupportedProtocols) {
    delete p;
  }
}

void cmServer::PopOne()
{
  this->Writing = false;
  if (this->Queue.empty()) {
    return;
  }

  Json::Reader reader;
  Json::Value value;
  const std::string input = this->Queue.front();
  this->Queue.erase(this->Queue.begin());

  if (!reader.parse(input, value)) {
    this->WriteParseError("Failed to parse JSON input.");
    return;
  }

  const cmServerRequest request(this, value[kTYPE_KEY].asString(),
                                value[kCOOKIE_KEY].asString(), value);

  if (request.Type == "") {
    cmServerResponse response(request);
    response.SetError("No type given in request.");
    this->WriteResponse(response);
    return;
  }

  this->WriteResponse(this->Protocol ? this->Protocol->Process(request)
                                     : this->SetProtocolVersion(request));
}

void cmServer::handleData(const std::string& data)
{
  this->DataBuffer += data;

  for (;;) {
    auto needle = this->DataBuffer.find('\n');

    if (needle == std::string::npos) {
      return;
    }
    std::string line = this->DataBuffer.substr(0, needle);
    const auto ls = line.size();
    if (ls > 1 && line.at(ls - 1) == '\r')
      line.erase(ls - 1, 1);
    this->DataBuffer.erase(this->DataBuffer.begin(),
                           this->DataBuffer.begin() + needle + 1);
    if (line == START_MAGIC) {
      this->JsonData.clear();
      continue;
    }
    if (line == END_MAGIC) {
      this->Queue.push_back(this->JsonData);
      this->JsonData.clear();
      if (!this->Writing) {
        this->PopOne();
      }
    } else {
      this->JsonData += line;
      this->JsonData += "\n";
    }
  }
}

void cmServer::RegisterProtocol(cmServerProtocol* protocol)
{
  auto version = protocol->ProtocolVersion();
  assert(version.first >= 0);
  assert(version.second >= 0);
  auto it = std::find_if(this->SupportedProtocols.begin(),
                         this->SupportedProtocols.end(),
                         [version](cmServerProtocol* p) {
                           return p->ProtocolVersion() == version;
                         });
  if (it == this->SupportedProtocols.end())
    this->SupportedProtocols.push_back(protocol);
}

void cmServer::PrintHello() const
{
  Json::Value hello = Json::objectValue;
  hello[kTYPE_KEY] = "hello";

  Json::Value& protocolVersions = hello["supportedProtocolVersions"] =
    Json::arrayValue;

  for (auto const& proto : this->SupportedProtocols) {
    auto version = proto->ProtocolVersion();
    Json::Value tmp = Json::objectValue;
    tmp["major"] = version.first;
    tmp["minor"] = version.second;
    protocolVersions.append(tmp);
  }

  this->WriteJsonObject(hello);
}

cmServerResponse cmServer::SetProtocolVersion(const cmServerRequest& request)
{
  if (request.Type != "handshake")
    return request.ReportError("Waiting for type \"handshake\".");

  Json::Value requestedProtocolVersion = request.Data["protocolVersion"];
  if (requestedProtocolVersion.isNull())
    return request.ReportError(
      "\"protocolVersion\" is required for \"handshake\".");

  if (!requestedProtocolVersion.isObject())
    return request.ReportError("\"protocolVersion\" must be a JSON object.");

  Json::Value majorValue = requestedProtocolVersion["major"];
  if (!majorValue.isInt())
    return request.ReportError("\"major\" must be set and an integer.");

  Json::Value minorValue = requestedProtocolVersion["minor"];
  if (!minorValue.isNull() && !minorValue.isInt())
    return request.ReportError("\"minor\" must be unset or an integer.");

  const int major = majorValue.asInt();
  const int minor = minorValue.isNull() ? -1 : minorValue.asInt();
  if (major < 0)
    return request.ReportError("\"major\" must be >= 0.");
  if (!minorValue.isNull() && minor < 0)
    return request.ReportError("\"minor\" must be >= 0 when set.");

  this->Protocol =
    this->FindMatchingProtocol(this->SupportedProtocols, major, minor);
  if (!this->Protocol) {
    return request.ReportError("Protocol version not supported.");
  }

  std::string errorMessage;
  if (!this->Protocol->Activate(request, &errorMessage)) {
    this->Protocol = CM_NULLPTR;
    return request.ReportError("Failed to activate protocol version: " +
                               errorMessage);
  }
  return request.Reply(Json::objectValue);
}

void cmServer::Serve()
{
  assert(!this->SupportedProtocols.empty());
  assert(!this->Protocol);

  this->Loop = uv_default_loop();

  if (uv_guess_handle(1) == UV_TTY) {
    uv_tty_init(this->Loop, &this->Input.tty, 0, 1);
    uv_tty_set_mode(&this->Input.tty, UV_TTY_MODE_NORMAL);
    this->Input.tty.data = this;
    InputStream = reinterpret_cast<uv_stream_t*>(&this->Input.tty);

    uv_tty_init(this->Loop, &this->Output.tty, 1, 0);
    uv_tty_set_mode(&this->Output.tty, UV_TTY_MODE_NORMAL);
    this->Output.tty.data = this;
    OutputStream = reinterpret_cast<uv_stream_t*>(&this->Output.tty);
  } else {
    uv_pipe_init(this->Loop, &this->Input.pipe, 0);
    uv_pipe_open(&this->Input.pipe, 0);
    this->Input.pipe.data = this;
    InputStream = reinterpret_cast<uv_stream_t*>(&this->Input.pipe);

    uv_pipe_init(this->Loop, &this->Output.pipe, 0);
    uv_pipe_open(&this->Output.pipe, 1);
    this->Output.pipe.data = this;
    OutputStream = reinterpret_cast<uv_stream_t*>(&this->Output.pipe);
  }

  this->PrintHello();

  uv_read_start(this->InputStream, alloc_buffer, read_stdin);

  uv_run(this->Loop, UV_RUN_DEFAULT);
}

void cmServer::WriteJsonObject(const Json::Value& jsonValue) const
{
  Json::FastWriter writer;

  std::string result = std::string("\n") + std::string(START_MAGIC) +
    std::string("\n") + writer.write(jsonValue) + std::string(END_MAGIC) +
    std::string("\n");

  this->Writing = true;
  write_data(this->OutputStream, result, on_stdout_write);
}

cmServerProtocol* cmServer::FindMatchingProtocol(
  const std::vector<cmServerProtocol*>& protocols, int major, int minor)
{
  cmServerProtocol* bestMatch = nullptr;
  for (auto protocol : protocols) {
    auto version = protocol->ProtocolVersion();
    if (major != version.first)
      continue;
    if (minor == version.second)
      return protocol;
    if (!bestMatch || bestMatch->ProtocolVersion().second < version.second)
      bestMatch = protocol;
  }
  return minor < 0 ? bestMatch : nullptr;
}

void cmServer::WriteProgress(const cmServerRequest& request, int min,
                             int current, int max,
                             const std::string& message) const
{
  assert(min <= current && current <= max);
  assert(message.length() != 0);

  Json::Value obj = Json::objectValue;
  obj[kTYPE_KEY] = PROGRESS_TYPE;
  obj[REPLY_TO_KEY] = request.Type;
  obj[kCOOKIE_KEY] = request.Cookie;
  obj["progressMessage"] = message;
  obj["progressMinimum"] = min;
  obj["progressMaximum"] = max;
  obj["progressCurrent"] = current;

  this->WriteJsonObject(obj);
}

void cmServer::WriteParseError(const std::string& message) const
{
  Json::Value obj = Json::objectValue;
  obj[kTYPE_KEY] = ERROR_TYPE;
  obj[ERROR_MESSAGE_KEY] = message;
  obj[REPLY_TO_KEY] = "";
  obj[kCOOKIE_KEY] = "";

  this->WriteJsonObject(obj);
}

void cmServer::WriteResponse(const cmServerResponse& response) const
{
  assert(response.IsComplete());

  Json::Value obj = response.Data();
  obj[kCOOKIE_KEY] = response.Cookie;
  obj[kTYPE_KEY] = response.IsError() ? ERROR_TYPE : REPLY_TYPE;
  obj[REPLY_TO_KEY] = response.Type;
  if (response.IsError()) {
    obj[ERROR_MESSAGE_KEY] = response.ErrorMessage();
  }

  this->WriteJsonObject(obj);
}
