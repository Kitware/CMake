// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dap/network.h"

#include "socket.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace {

class Impl : public dap::net::Server {
 public:
  Impl() : stopped{true} {}

  ~Impl() { stop(); }

  bool start(int port,
             const OnConnect& onConnect,
             const OnError& onError) override {
    std::unique_lock<std::mutex> lock(mutex);
    stopWithLock();
    socket = std::unique_ptr<dap::Socket>(
        new dap::Socket("localhost", std::to_string(port).c_str()));

    if (!socket->isOpen()) {
      onError("Failed to open socket");
      return false;
    }

    stopped = false;
    thread = std::thread([=] {
      while (true) {
        if (auto rw = socket->accept()) {
          onConnect(rw);
          continue;
        }
        if (!stopped) {
          onError("Failed to accept connection");
        }
        break;
      };
    });

    return true;
  }

  void stop() override {
    std::unique_lock<std::mutex> lock(mutex);
    stopWithLock();
  }

 private:
  bool isRunning() { return !stopped; }

  void stopWithLock() {
    if (!stopped.exchange(true)) {
      socket->close();
      thread.join();
    }
  }

  std::mutex mutex;
  std::thread thread;
  std::unique_ptr<dap::Socket> socket;
  std::atomic<bool> stopped;
  OnError errorHandler;
};

}  // anonymous namespace

namespace dap {
namespace net {

std::unique_ptr<Server> Server::create() {
  return std::unique_ptr<Server>(new Impl());
}

std::shared_ptr<ReaderWriter> connect(const char* addr,
                                      int port,
                                      uint32_t timeoutMillis) {
  return Socket::connect(addr, std::to_string(port).c_str(), timeoutMillis);
}

}  // namespace net
}  // namespace dap
