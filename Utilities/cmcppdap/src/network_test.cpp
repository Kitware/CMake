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
#include "dap/io.h"

#include "chan.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <chrono>
#include <thread>

namespace {

constexpr int port = 19021;

bool write(const std::shared_ptr<dap::Writer>& w, const std::string& s) {
  return w->write(s.data(), s.size()) && w->write("\0", 1);
}

std::string read(const std::shared_ptr<dap::Reader>& r) {
  char c;
  std::string s;
  while (r->read(&c, sizeof(c)) > 0) {
    if (c == '\0') {
      return s;
    }
    s += c;
  }
  return r->isOpen() ? "<read failed>" : "<stream closed>";
}

}  // anonymous namespace

TEST(Network, ClientServer) {
  dap::Chan<bool> done;
  auto server = dap::net::Server::create();
  if (!server->start(
          port,
          [&](const std::shared_ptr<dap::ReaderWriter>& rw) {
            ASSERT_EQ(read(rw), "client to server");
            ASSERT_TRUE(write(rw, "server to client"));
            done.put(true);
          },
          [&](const char* err) { FAIL() << "Server error: " << err; })) {
    FAIL() << "Couldn't start server";
    return;
  }

  for (int i = 0; i < 5; i++) {
    auto client = dap::net::connect("localhost", port);
    ASSERT_NE(client, nullptr) << "Failed to connect client " << i;
    ASSERT_TRUE(write(client, "client to server"));
    ASSERT_EQ(read(client), "server to client");
    done.take();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  server.reset();
}

TEST(Network, ServerRepeatStopAndRestart) {
  dap::Chan<bool> done;
  auto onConnect = [&](const std::shared_ptr<dap::ReaderWriter>& rw) {
    ASSERT_EQ(read(rw), "client to server");
    ASSERT_TRUE(write(rw, "server to client"));
    done.put(true);
  };
  auto onError = [&](const char* err) { FAIL() << "Server error: " << err; };

  auto server = dap::net::Server::create();
  if (!server->start(port, onConnect, onError)) {
    FAIL() << "Couldn't start server";
    return;
  }

  server->stop();
  server->stop();
  server->stop();

  if (!server->start(port, onConnect, onError)) {
    FAIL() << "Couldn't restart server";
    return;
  }

  auto client = dap::net::connect("localhost", port);
  ASSERT_NE(client, nullptr) << "Failed to connect";
  ASSERT_TRUE(write(client, "client to server"));
  ASSERT_EQ(read(client), "server to client");
  done.take();

  server->stop();
  server->stop();
  server->stop();

  server.reset();
}
