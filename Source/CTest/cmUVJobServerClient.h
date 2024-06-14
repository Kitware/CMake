/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <memory>

#include <cm/optional>

#include <cm3p/uv.h>

/** \class cmUVJobServerClient
 * \brief Job server client that can integrate with a libuv event loop.
 *
 * Use the \a Connect method to connect to an ambient job server as
 * described by the MAKEFLAGS environment variable, if any.  Request
 * a token using the \a RequestToken method.  The \a onToken callback
 * will be invoked asynchronously when the token is received.  Act
 * on the token, and then use \a ReleaseToken to release it.
 *
 * The job server protocol states that a client process implicitly
 * has one free token available, corresponding to the token its
 * parent used to start it.  \a cmUVJobServerClient will use the
 * implicit token whenever it is available instead of requesting
 * an explicit token from the job server.  However, clients of
 * this class must still request and receive the token before
 * acting on it, and cannot assume that it is always held.
 *
 * If the job server connection breaks, \a onDisconnect will be
 * called with the libuv error.  No further tokens can be received
 * from the job server, but progress can still be made serially
 * using the implicit token.
 */
class cmUVJobServerClient
{
public:
  class Impl;

private:
  std::unique_ptr<Impl> Impl_;

  cmUVJobServerClient(std::unique_ptr<Impl> impl);

public:
  /**
   * Disconnect from the job server.
   */
  ~cmUVJobServerClient();

  cmUVJobServerClient(cmUVJobServerClient&&) noexcept;
  cmUVJobServerClient(cmUVJobServerClient const&) = delete;
  cmUVJobServerClient& operator=(cmUVJobServerClient&&) noexcept;
  cmUVJobServerClient& operator=(cmUVJobServerClient const&) = delete;

  /**
   * Request a token from the job server.
   * When the token is held, the \a onToken callback will be invoked.
   */
  void RequestToken();

  /**
   * Release a token to the job server.
   * This may be called only after a corresponding \a onToken callback.
   */
  void ReleaseToken();

  /**
   * Get the number of implicit and explicit tokens currently held.
   * This is the number of times \a onToken has been called but not
   * yet followed by a call to \a ReleaseToken.
   * This is meant for testing and debugging.
   */
  int GetHeldTokens() const;

  /**
   * Get the number of explicit tokens currently requested from the
   * job server but not yet received.  If the implicit token becomes
   * available, it is used in place of a requested token, and this
   * is decremented without receiving an explicit token.
   * This is meant for testing and debugging.
   */
  int GetNeedTokens() const;

  /**
   * Connect to an ambient job server, if any.
   * \param loop          The libuv event loop on which to schedule events.
   * \param onToken       Function to call when a new token is held.
   * \param onDisconnect  Function to call on disconnect, with libuv error.
   * \returns             Connected instance, or cm::nullopt.
   */
  static cm::optional<cmUVJobServerClient> Connect(
    uv_loop_t& loop, std::function<void()> onToken,
    std::function<void(int)> onDisconnect);
};
