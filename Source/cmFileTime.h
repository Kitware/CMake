/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmFileTime
 * \brief Abstract file modification time with support for comparison with
 *        other file modification times.
 */
class cmFileTime
{
public:
  using TimeType = long long;
  // unit time per second
#if !defined(_WIN32) || defined(__CYGWIN__)
  // unit time is one nanosecond
  static constexpr TimeType UtPerS = 1000000000;
#else
  // unit time is 100 nanosecond
  static constexpr TimeType UtPerS = 10000000;
#endif
  cmFileTime() = default;
  ~cmFileTime() = default;
  cmFileTime(const cmFileTime&) = default;
  cmFileTime& operator=(const cmFileTime&) = default;

  /**
   * @brief Loads the file time of fileName from the file system
   * @return true on success
   */
  bool Load(std::string const& fileName);

  /**
   * @brief Return true if this is older than ftm
   */
  bool Older(cmFileTime const& ftm) const
  {
    return (this->Time - ftm.Time) < 0;
  }

  /**
   * @brief Return true if this is newer than ftm
   */
  bool Newer(cmFileTime const& ftm) const
  {
    return (ftm.Time - this->Time) < 0;
  }

  /**
   * @brief Return true if this is the same as ftm
   */
  bool Equal(cmFileTime const& ftm) const { return this->Time == ftm.Time; }

  /**
   * @brief Return true if this is not the same as ftm
   */
  bool Differ(cmFileTime const& ftm) const { return this->Time != ftm.Time; }

  /**
   * @brief Compare file modification times.
   * @return -1, 0, +1 for this older, same, or newer than ftm.
   */
  int Compare(cmFileTime const& ftm) const
  {
    TimeType const diff = this->Time - ftm.Time;
    if (diff == 0) {
      return 0;
    }
    return (diff < 0) ? -1 : 1;
  }

  // -- Comparison in second resolution

  /**
   * @brief Return true if this is at least a second older than ftm
   */
  bool OlderS(cmFileTime const& ftm) const
  {
    return (ftm.Time - this->Time) >= cmFileTime::UtPerS;
  }

  /**
   * @brief Return true if this is at least a second newer than ftm
   */
  bool NewerS(cmFileTime const& ftm) const
  {
    return (this->Time - ftm.Time) >= cmFileTime::UtPerS;
  }

  /**
   * @brief Return true if this is within the same second as ftm
   */
  bool EqualS(cmFileTime const& ftm) const
  {
    TimeType diff = this->Time - ftm.Time;
    if (diff < 0) {
      diff = -diff;
    }
    return (diff < cmFileTime::UtPerS);
  }

  /**
   * @brief Return true if this is older or newer than ftm by at least a second
   */
  bool DifferS(cmFileTime const& ftm) const
  {
    TimeType diff = this->Time - ftm.Time;
    if (diff < 0) {
      diff = -diff;
    }
    return (diff >= cmFileTime::UtPerS);
  }

  /**
   * @brief Compare file modification times.
   * @return -1: this at least a second older, 0: this within the same second
   *         as ftm, +1: this at least a second newer than ftm.
   */
  int CompareS(cmFileTime const& ftm) const
  {
    TimeType const diff = this->Time - ftm.Time;
    if (diff <= -cmFileTime::UtPerS) {
      return -1;
    }
    if (diff >= cmFileTime::UtPerS) {
      return 1;
    }
    return 0;
  }

  /**
   * @brief The file modification time in unit time per second
   */
  TimeType GetTime() const { return this->Time; }

private:
  TimeType Time = 0;
};
