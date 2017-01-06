/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cm_codecvt.hxx"
#include <limits>

#if defined(_WIN32)
#include <windows.h>
#undef max
#include <cmsys/Encoding.hxx>
#endif

codecvt::codecvt(Encoding e)
  : m_lastState(0)
#if defined(_WIN32)
  , m_codepage(0)
#endif
{
  switch (e) {
    case codecvt::ANSI:
#if defined(_WIN32)
      m_noconv = false;
      m_codepage = CP_ACP;
      break;
#endif
    // We don't know which ANSI encoding to use for other platforms than
    // Windows so we don't do any conversion there
    case codecvt::UTF8:
    // Assume internal encoding is UTF-8
    case codecvt::None:
    // No encoding
    default:
      m_noconv = true;
  }
}

codecvt::~codecvt(){};

bool codecvt::do_always_noconv() const throw()
{
  return m_noconv;
};

std::codecvt_base::result codecvt::do_out(mbstate_t& state, const char* from,
                                          const char* from_end,
                                          const char*& from_next, char* to,
                                          char* to_end, char*& to_next) const
{
  if (m_noconv) {
    return noconv;
  }
  std::codecvt_base::result res = error;
#if defined(_WIN32)
  from_next = from;
  to_next = to;
  bool convert = true;
  size_t count = from_end - from;
  const char* data = from;
  unsigned int& stateId = reinterpret_cast<unsigned int&>(state);
  if (count == 0) {
    return codecvt::ok;
  } else if (count == 1) {
    if (stateId == 0) {
      // decode first byte for UTF-8
      if ((*from & 0xF8) == 0xF0 || // 1111 0xxx; 4 bytes for codepoint
          (*from & 0xF0) == 0xE0 || // 1110 xxxx; 3 bytes for codepoint
          (*from & 0xE0) == 0xC0)   // 110x xxxx; 2 bytes for codepoint
      {
        stateId = findStateId();
        codecvt::State& s = m_states.at(stateId - 1);
        s.bytes[0] = *from;
        convert = false;
        if ((*from & 0xF8) == 0xF0) {
          s.totalBytes = 4;
        } else if ((*from & 0xF0) == 0xE0) {
          s.totalBytes = 3;
        } else if ((*from & 0xE0) == 0xC0) {
          s.totalBytes = 2;
        }
        s.bytesLeft = s.totalBytes - 1;
      };
      // else 1 byte for codepoint
    } else {
      codecvt::State& s = m_states.at(stateId - 1);
      s.bytes[s.totalBytes - s.bytesLeft] = *from;
      s.bytesLeft--;
      data = s.bytes;
      count = s.totalBytes - s.bytesLeft;
      if ((*from & 0xC0) == 0x80) { // 10xx xxxx
        convert = s.bytesLeft == 0;
      } else {
        // invalid multi-byte
        convert = true;
      }
      if (convert) {
        s.used = false;
        if (stateId == m_lastState) {
          m_lastState--;
        }
        stateId = 0;
      }
    }
    if (convert) {
      std::wstring wide = cmsys::Encoding::ToWide(std::string(data, count));
      int r = WideCharToMultiByte(m_codepage, 0, wide.c_str(),
                                  static_cast<int>(wide.size()), to,
                                  to_end - to, NULL, NULL);
      if (r > 0) {
        from_next = from_end;
        to_next = to + r;
        res = ok;
      }
    } else {
      res = partial;
      from_next = from_end;
      to_next = to;
    }
  }
#else
  static_cast<void>(state);
  static_cast<void>(from);
  static_cast<void>(from_end);
  static_cast<void>(from_next);
  static_cast<void>(to);
  static_cast<void>(to_end);
  static_cast<void>(to_next);
  res = codecvt::noconv;
#endif
  return res;
};

std::codecvt_base::result codecvt::do_unshift(mbstate_t& state, char* to,
                                              char* to_end,
                                              char*& to_next) const
{
  std::codecvt_base::result res = error;
  to_next = to;
#if defined(_WIN32)
  unsigned int& stateId = reinterpret_cast<unsigned int&>(state);
  if (stateId > 0) {
    codecvt::State& s = m_states.at(stateId - 1);
    s.used = false;
    if (stateId == m_lastState) {
      m_lastState--;
    }
    stateId = 0;
    std::wstring wide = cmsys::Encoding::ToWide(
      std::string(s.bytes, s.totalBytes - s.bytesLeft));
    int r = WideCharToMultiByte(m_codepage, 0, wide.c_str(),
                                static_cast<int>(wide.size()), to, to_end - to,
                                NULL, NULL);
    if (r > 0) {
      to_next = to + r;
      res = ok;
    }
  } else {
    res = ok;
  }
#else
  static_cast<void>(state);
  static_cast<void>(to_end);
  res = ok;
#endif
  return res;
};

int codecvt::do_max_length() const throw()
{
  return 4;
};

int codecvt::do_encoding() const throw()
{
  return 0;
};

unsigned int codecvt::findStateId() const
{
  unsigned int stateId = 0;
  bool add = false;
  const unsigned int maxSize = std::numeric_limits<unsigned int>::max();
  if (m_lastState >= maxSize) {
    m_lastState = 0;
  }
  if (m_states.size() <= m_lastState) {
    add = true;
  } else {
    unsigned int i = m_lastState;
    while (i < maxSize) {
      codecvt::State& s = m_states.at(i);
      i++;
      if (!s.used) {
        m_lastState = i;
        stateId = m_lastState;
        s.used = true;
        s.totalBytes = 0;
        s.bytesLeft = 0;
        break;
      }
      if (i >= m_states.size()) {
        i = 0;
      }
      if (i == m_lastState) {
        add = true;
        break;
      }
    }
  };
  if (add) {
    codecvt::State s = { true, 0, 0, { 0, 0, 0, 0 } };
    m_states.push_back(s);
    m_lastState = (unsigned int)m_states.size();
    stateId = m_lastState;
  }
  return stateId;
};
