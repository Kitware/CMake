/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileTime_h
#define cmFileTime_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmFileTime
 * \brief Abstract file modification time with support for comparison with
 *        other file modification times.
 */
class cmFileTime
{
public:
  using NSC = long long;
  static constexpr NSC NsPerS = 1000000000;

  cmFileTime() = default;
  ~cmFileTime() = default;

  /**
   * @brief Loads the file time of fileName from the file system
   * @return true on success
   */
  bool Load(std::string const& fileName);

  /**
   * @brief Return true if this is older than ftm
   */
  bool Older(cmFileTime const& ftm) const { return (this->NS - ftm.NS) < 0; }

  /**
   * @brief Return true if this is newer than ftm
   */
  bool Newer(cmFileTime const& ftm) const { return (ftm.NS - this->NS) < 0; }

  /**
   * @brief Return true if this is the same as ftm
   */
  bool Equal(cmFileTime const& ftm) const { return this->NS == ftm.NS; }

  /**
   * @brief Return true if this is not the same as ftm
   */
  bool Differ(cmFileTime const& ftm) const { return this->NS != ftm.NS; }

  /**
   * @brief Compare file modification times.
   * @return -1, 0, +1 for this older, same, or newer than ftm.
   */
  int Compare(cmFileTime const& ftm) const
  {
    NSC const diff = this->NS - ftm.NS;
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
    return (ftm.NS - this->NS) >= cmFileTime::NsPerS;
  }

  /**
   * @brief Return true if this is at least a second newer than ftm
   */
  bool NewerS(cmFileTime const& ftm) const
  {
    return (this->NS - ftm.NS) >= cmFileTime::NsPerS;
  }

  /**
   * @brief Return true if this is within the same second as ftm
   */
  bool EqualS(cmFileTime const& ftm) const
  {
    NSC diff = this->NS - ftm.NS;
    if (diff < 0) {
      diff = -diff;
    }
    return (diff < cmFileTime::NsPerS);
  }

  /**
   * @brief Return true if this is older or newer than ftm by at least a second
   */
  bool DifferS(cmFileTime const& ftm) const
  {
    NSC diff = this->NS - ftm.NS;
    if (diff < 0) {
      diff = -diff;
    }
    return (diff >= cmFileTime::NsPerS);
  }

  /**
   * @brief Compare file modification times.
   * @return -1: this at least a second older, 0: this within the same second
   *         as ftm, +1: this at least a second newer than ftm.
   */
  int CompareS(cmFileTime const& ftm) const
  {
    NSC const diff = this->NS - ftm.NS;
    if (diff <= -cmFileTime::NsPerS) {
      return -1;
    }
    if (diff >= cmFileTime::NsPerS) {
      return 1;
    }
    return 0;
  }

  /**
   * @brief The file modification time in nanoseconds
   */
  NSC GetNS() const { return this->NS; }

private:
  NSC NS = 0;
};

#endif
