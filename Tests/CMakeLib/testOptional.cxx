#include <iostream>
#include <vector>

#include <cm/optional>
#include <cm/utility>

class EventLogger;

class Event
{
public:
  enum EventType
  {
    DEFAULT_CONSTRUCT,
    COPY_CONSTRUCT,
    MOVE_CONSTRUCT,
    VALUE_CONSTRUCT,

    DESTRUCT,

    COPY_ASSIGN,
    MOVE_ASSIGN,
    VALUE_ASSIGN,

    REFERENCE,
    CONST_REFERENCE,
    RVALUE_REFERENCE,
    CONST_RVALUE_REFERENCE,

    SWAP,

    COMPARE_EE_EQ,
    COMPARE_EE_NE,
    COMPARE_EE_LT,
    COMPARE_EE_LE,
    COMPARE_EE_GT,
    COMPARE_EE_GE,
  };

  EventType Type;
  const EventLogger* Logger1;
  const EventLogger* Logger2;
  int Value;

  bool operator==(const Event& other) const;
  bool operator!=(const Event& other) const;
};

bool Event::operator==(const Event& other) const
{
  return this->Type == other.Type && this->Logger1 == other.Logger1 &&
    this->Logger2 == other.Logger2 && this->Value == other.Value;
}

bool Event::operator!=(const Event& other) const
{
  return !(*this == other);
}

static std::vector<Event> events;

class EventLogger
{
public:
  EventLogger();
  EventLogger(const EventLogger& other);
  EventLogger(EventLogger&& other);
  EventLogger(int value);

  ~EventLogger();

  EventLogger& operator=(const EventLogger& other);
  EventLogger& operator=(EventLogger&& other);
  EventLogger& operator=(int value);

  void Reference() &;
  void Reference() const&;
  void Reference() &&;
  void Reference() const&&;

  int Value = 0;
};

class NoMoveAssignEventLogger : public EventLogger
{
public:
  using EventLogger::EventLogger;

  NoMoveAssignEventLogger(const NoMoveAssignEventLogger&) = default;
  NoMoveAssignEventLogger(NoMoveAssignEventLogger&&) = default;

  NoMoveAssignEventLogger& operator=(const NoMoveAssignEventLogger&) = default;
  NoMoveAssignEventLogger& operator=(NoMoveAssignEventLogger&&) = delete;
};

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

// Certain builds of GCC generate false -Wmaybe-uninitialized warnings when
// doing a release build with the system version of std::optional. These
// warnings do not manifest when using our own cm::optional implementation.
// Silence these false warnings.
#if defined(__GNUC__) && !defined(__clang__)
#  define BEGIN_IGNORE_UNINITIALIZED                                          \
    _Pragma("GCC diagnostic push")                                            \
      _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#  define END_IGNORE_UNINITIALIZED _Pragma("GCC diagnostic pop")
#else
#  define BEGIN_IGNORE_UNINITIALIZED
#  define END_IGNORE_UNINITIALIZED
#endif

static void swap(EventLogger& e1, EventLogger& e2)
{
  BEGIN_IGNORE_UNINITIALIZED
  events.push_back({ Event::SWAP, &e1, &e2, e2.Value });
  END_IGNORE_UNINITIALIZED
  auto tmp = e1.Value;
  e1.Value = e2.Value;
  e2.Value = tmp;
}

EventLogger::EventLogger()
  : Value(0)
{
  events.push_back({ Event::DEFAULT_CONSTRUCT, this, nullptr, 0 });
}

EventLogger::EventLogger(const EventLogger& other)
  : Value(other.Value)
{
  events.push_back({ Event::COPY_CONSTRUCT, this, &other, other.Value });
}

BEGIN_IGNORE_UNINITIALIZED
EventLogger::EventLogger(EventLogger&& other)
  : Value(other.Value)
{
  events.push_back({ Event::MOVE_CONSTRUCT, this, &other, other.Value });
}
END_IGNORE_UNINITIALIZED

EventLogger::EventLogger(int value)
  : Value(value)
{
  events.push_back({ Event::VALUE_CONSTRUCT, this, nullptr, value });
}

EventLogger::~EventLogger()
{
  BEGIN_IGNORE_UNINITIALIZED
  events.push_back({ Event::DESTRUCT, this, nullptr, this->Value });
  END_IGNORE_UNINITIALIZED
}

EventLogger& EventLogger::operator=(const EventLogger& other)
{
  events.push_back({ Event::COPY_ASSIGN, this, &other, other.Value });
  this->Value = other.Value;
  return *this;
}

EventLogger& EventLogger::operator=(EventLogger&& other)
{
  events.push_back({ Event::MOVE_ASSIGN, this, &other, other.Value });
  this->Value = other.Value;
  return *this;
}

EventLogger& EventLogger::operator=(int value)
{
  events.push_back({ Event::VALUE_ASSIGN, this, nullptr, value });
  this->Value = value;
  return *this;
}

static bool operator==(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_EQ, &lhs, &rhs, lhs.Value });
  return lhs.Value == rhs.Value;
}

static bool operator!=(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_NE, &lhs, &rhs, lhs.Value });
  return lhs.Value != rhs.Value;
}

static bool operator<(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_LT, &lhs, &rhs, lhs.Value });
  return lhs.Value < rhs.Value;
}

static bool operator<=(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_LE, &lhs, &rhs, lhs.Value });
  return lhs.Value <= rhs.Value;
}

static bool operator>(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_GT, &lhs, &rhs, lhs.Value });
  return lhs.Value > rhs.Value;
}

static bool operator>=(const EventLogger& lhs, const EventLogger& rhs)
{
  events.push_back({ Event::COMPARE_EE_GE, &lhs, &rhs, lhs.Value });
  return lhs.Value >= rhs.Value;
}

void EventLogger::Reference() &
{
  events.push_back({ Event::REFERENCE, this, nullptr, this->Value });
}

void EventLogger::Reference() const&
{
  events.push_back({ Event::CONST_REFERENCE, this, nullptr, this->Value });
}

void EventLogger::Reference() &&
{
  events.push_back({ Event::RVALUE_REFERENCE, this, nullptr, this->Value });
}

void EventLogger::Reference() const&&
{
  events.push_back(
    { Event::CONST_RVALUE_REFERENCE, this, nullptr, this->Value });
}

static bool testDefaultConstruct(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o{};

  expected = {};
  return true;
}

static bool testNulloptConstruct(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o{ cm::nullopt };

  expected = {};
  return true;
}

static bool testValueConstruct(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o{ 4 };

  expected = {
    { Event::VALUE_CONSTRUCT, &*o, nullptr, 4 },
    { Event::DESTRUCT, &*o, nullptr, 4 },
  };
  return true;
}

static bool testInPlaceConstruct(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o1{ cm::in_place, 4 };
  const cm::optional<EventLogger> o2{ cm::in_place_t{}, 4 };

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 4 },
    { Event::DESTRUCT, &*o2, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return true;
}

static bool testCopyConstruct(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{ o1 };
  const cm::optional<EventLogger> o3{};
  const cm::optional<EventLogger> o4{ o3 };

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::COPY_CONSTRUCT, &*o2, &o1.value(), 4 },
    { Event::DESTRUCT, &*o2, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return true;
}

static bool testMoveConstruct(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{ std::move(o1) };
  cm::optional<EventLogger> o3{};
  const cm::optional<EventLogger> o4{ std::move(o3) };

#ifndef __clang_analyzer__ /* cplusplus.Move */
  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::MOVE_CONSTRUCT, &*o2, &*o1, 4 },
    { Event::DESTRUCT, &*o2, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
#endif
  return true;
}

static bool testNulloptAssign(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  auto const* v1 = &*o1;
  o1 = cm::nullopt;
  cm::optional<EventLogger> o2{};
  o2 = cm::nullopt;

  expected = {
    { Event::VALUE_CONSTRUCT, v1, nullptr, 4 },
    { Event::DESTRUCT, v1, nullptr, 4 },
  };
  return true;
}

static bool testCopyAssign(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{};
  const cm::optional<EventLogger> o2{ 4 };
  auto const* v2 = &*o2;
  o1 = o2;
  auto const* v1 = &*o1;
  const cm::optional<EventLogger> o3{ 5 };
  auto const* v3 = &*o3;
  o1 = o3;
  const cm::optional<EventLogger> o4{};
  o1 = o4;
  o1 = o4; // Intentionally duplicated to test assigning an empty optional to
  // an empty optional

  cm::optional<NoMoveAssignEventLogger> o5{ 1 };
  auto const* v5 = &*o5;
  const cm::optional<NoMoveAssignEventLogger> o6{ 2 };
  auto const* v6 = &*o6;
  o5 = std::move(o6);
  const NoMoveAssignEventLogger e7{ 3 };
  o5 = std::move(e7);

  expected = {
    { Event::VALUE_CONSTRUCT, v2, nullptr, 4 },
    { Event::COPY_CONSTRUCT, v1, v2, 4 },
    { Event::VALUE_CONSTRUCT, v3, nullptr, 5 },
    { Event::COPY_ASSIGN, v1, v3, 5 },
    { Event::DESTRUCT, v1, nullptr, 5 },
    { Event::VALUE_CONSTRUCT, v5, nullptr, 1 },
    { Event::VALUE_CONSTRUCT, v6, nullptr, 2 },
    { Event::COPY_ASSIGN, v5, v6, 2 },
    { Event::VALUE_CONSTRUCT, &e7, nullptr, 3 },
    { Event::COPY_ASSIGN, v5, &e7, 3 },
    { Event::DESTRUCT, &e7, nullptr, 3 },
    { Event::DESTRUCT, v6, nullptr, 2 },
    { Event::DESTRUCT, v5, nullptr, 3 },
    { Event::DESTRUCT, v3, nullptr, 5 },
    { Event::DESTRUCT, v2, nullptr, 4 },
  };
  return true;
}

static bool testMoveAssign(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{};
  cm::optional<EventLogger> o2{ 4 };
  auto const* v2 = &*o2;
  o1 = std::move(o2);
  auto const* v1 = &*o1;
  cm::optional<EventLogger> o3{ 5 };
  auto const* v3 = &*o3;
  o1 = std::move(o3);
  cm::optional<EventLogger> o4{};
  o1 = std::move(o4);

  expected = {
    { Event::VALUE_CONSTRUCT, v2, nullptr, 4 },
    { Event::MOVE_CONSTRUCT, v1, v2, 4 },
    { Event::VALUE_CONSTRUCT, v3, nullptr, 5 },
    { Event::MOVE_ASSIGN, v1, v3, 5 },
    { Event::DESTRUCT, v1, nullptr, 5 },
    { Event::DESTRUCT, v3, nullptr, 5 },
    { Event::DESTRUCT, v2, nullptr, 4 },
  };
  return true;
}

static bool testPointer(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{ 5 };

  o1->Reference();
  o2->Reference();

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 5 },
    { Event::REFERENCE, &*o1, nullptr, 4 },
    { Event::CONST_REFERENCE, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return true;
}

#if !__GNUC__ || __GNUC__ > 4
#  define ALLOW_CONST_RVALUE
#endif

static bool testDereference(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  auto const* v1 = &*o1;
  const cm::optional<EventLogger> o2{ 5 };
  auto const* v2 = &*o2;

  (*o1).Reference();
  (*o2).Reference();
  (*std::move(o1)).Reference();
#ifdef ALLOW_CONST_RVALUE
  (*std::move(o2)).Reference(); // Broken in GCC 4.9.0. Sigh...
#endif

  expected = {
    { Event::VALUE_CONSTRUCT, v1, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, v2, nullptr, 5 },
    { Event::REFERENCE, v1, nullptr, 4 },
    { Event::CONST_REFERENCE, v2, nullptr, 5 },
    { Event::RVALUE_REFERENCE, v1, nullptr, 4 },
#ifdef ALLOW_CONST_RVALUE
    { Event::CONST_RVALUE_REFERENCE, v2, nullptr, 5 },
#endif
    { Event::DESTRUCT, v2, nullptr, 5 },
    { Event::DESTRUCT, v1, nullptr, 4 },
  };
  return true;
}

static bool testHasValue(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{};

  ASSERT_TRUE(o1.has_value());
  ASSERT_TRUE(o1);
  ASSERT_TRUE(!o2.has_value());
  ASSERT_TRUE(!o2);

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return true;
}

static bool testValue(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{ 5 };
  cm::optional<EventLogger> o3{};
  const cm::optional<EventLogger> o4{};

  o1.value().Reference();
  o2.value().Reference();

  bool thrown = false;
  try {
    (void)o3.value();
  } catch (cm::bad_optional_access&) {
    thrown = true;
  }
  ASSERT_TRUE(thrown);

  thrown = false;
  try {
    (void)o4.value();
  } catch (cm::bad_optional_access&) {
    thrown = true;
  }
  ASSERT_TRUE(thrown);

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 5 },
    { Event::REFERENCE, &*o1, nullptr, 4 },
    { Event::CONST_REFERENCE, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return true;
}

static bool testValueOr()
{
  const cm::optional<EventLogger> o1{ 4 };
  cm::optional<EventLogger> o2{ 5 };
  const cm::optional<EventLogger> o3{};
  cm::optional<EventLogger> o4{};

  EventLogger e1{ 6 };
  EventLogger e2{ 7 };
  EventLogger e3{ 8 };
  EventLogger e4{ 9 };

  EventLogger r1 = o1.value_or(e1);
  ASSERT_TRUE(r1.Value == 4);
  EventLogger r2 = std::move(o2).value_or(e2);
  ASSERT_TRUE(r2.Value == 5);
  EventLogger r3 = o3.value_or(e3);
  ASSERT_TRUE(r3.Value == 8);
  EventLogger r4 = std::move(o4).value_or(e4);
  ASSERT_TRUE(r4.Value == 9);

  return true;
}

static bool testComparison(std::vector<Event>& expected)
{
  const cm::optional<EventLogger> o1{ 1 };
  const cm::optional<EventLogger> o2{ 2 };
  const cm::optional<EventLogger> o3{ 2 };
  const cm::optional<EventLogger> o4{};
  const cm::optional<EventLogger> o5{};
  const EventLogger e1{ 2 };

  ASSERT_TRUE(!(o1 == o2) && o1 != o2);
  ASSERT_TRUE(o1 < o2 && !(o1 >= o2));
  ASSERT_TRUE(!(o1 > o2) && o1 <= o2);

  ASSERT_TRUE(o2 == o3 && !(o2 != o3));
  ASSERT_TRUE(!(o2 < o3) && o2 >= o3);
  ASSERT_TRUE(!(o2 > o3) && o2 <= o3);

  ASSERT_TRUE(!(o3 == o4) && o3 != o4);
  ASSERT_TRUE(!(o3 < o4) && o3 >= o4);
  ASSERT_TRUE(o3 > o4 && !(o3 <= o4));

  ASSERT_TRUE(o4 == o5 && !(o4 != o5));
  ASSERT_TRUE(!(o4 < o5) && o4 >= o5);
  ASSERT_TRUE(!(o4 > o5) && o4 <= o5);

  ASSERT_TRUE(!(o1 == cm::nullopt) && o1 != cm::nullopt);
  ASSERT_TRUE(!(o1 < cm::nullopt) && o1 >= cm::nullopt);
  ASSERT_TRUE(o1 > cm::nullopt && !(o1 <= cm::nullopt));

  ASSERT_TRUE(!(cm::nullopt == o1) && cm::nullopt != o1);
  ASSERT_TRUE(cm::nullopt < o1 && !(cm::nullopt >= o1));
  ASSERT_TRUE(!(cm::nullopt > o1) && cm::nullopt <= o1);

  ASSERT_TRUE(o4 == cm::nullopt && !(o4 != cm::nullopt));
  ASSERT_TRUE(!(o4 < cm::nullopt) && o4 >= cm::nullopt);
  ASSERT_TRUE(!(o4 > cm::nullopt) && o4 <= cm::nullopt);

  ASSERT_TRUE(cm::nullopt == o4 && !(cm::nullopt != o4));
  ASSERT_TRUE(!(cm::nullopt < o4) && cm::nullopt >= o4);
  ASSERT_TRUE(!(cm::nullopt > o4) && cm::nullopt <= o4);

  ASSERT_TRUE(!(o1 == e1) && o1 != e1);
  ASSERT_TRUE(o1 < e1 && !(o1 >= e1));
  ASSERT_TRUE(!(o1 > e1) && o1 <= e1);

  ASSERT_TRUE(o2 == e1 && !(o2 != e1));
  ASSERT_TRUE(!(o2 < e1) && o2 >= e1);
  ASSERT_TRUE(!(o2 > e1) && o2 <= e1);

  ASSERT_TRUE(!(o4 == e1) && o4 != e1);
  ASSERT_TRUE(o4 < e1 && !(o4 >= e1));
  ASSERT_TRUE(!(o4 > e1) && o4 <= e1);

  ASSERT_TRUE(!(e1 == o1) && e1 != o1);
  ASSERT_TRUE(!(e1 < o1) && e1 >= o1);
  ASSERT_TRUE(e1 > o1 && !(e1 <= o1));

  ASSERT_TRUE(e1 == o2 && !(e1 != o2));
  ASSERT_TRUE(!(e1 < o2) && e1 >= o2);
  ASSERT_TRUE(!(e1 > o2) && e1 <= o2);

  ASSERT_TRUE(!(e1 == o4) && e1 != o4);
  ASSERT_TRUE(!(e1 < o4) && e1 >= o4);
  ASSERT_TRUE(e1 > o4 && !(e1 <= o4));

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 1 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 2 },
    { Event::VALUE_CONSTRUCT, &*o3, nullptr, 2 },
    { Event::VALUE_CONSTRUCT, &e1, nullptr, 2 },
    { Event::COMPARE_EE_EQ, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_NE, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_LT, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_GE, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_GT, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_LE, &*o1, &*o2, 1 },
    { Event::COMPARE_EE_EQ, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_NE, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_LT, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_GE, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_GT, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_LE, &*o2, &*o3, 2 },
    { Event::COMPARE_EE_EQ, &*o1, &e1, 1 },
    { Event::COMPARE_EE_NE, &*o1, &e1, 1 },
    { Event::COMPARE_EE_LT, &*o1, &e1, 1 },
    { Event::COMPARE_EE_GE, &*o1, &e1, 1 },
    { Event::COMPARE_EE_GT, &*o1, &e1, 1 },
    { Event::COMPARE_EE_LE, &*o1, &e1, 1 },
    { Event::COMPARE_EE_EQ, &*o2, &e1, 2 },
    { Event::COMPARE_EE_NE, &*o2, &e1, 2 },
    { Event::COMPARE_EE_LT, &*o2, &e1, 2 },
    { Event::COMPARE_EE_GE, &*o2, &e1, 2 },
    { Event::COMPARE_EE_GT, &*o2, &e1, 2 },
    { Event::COMPARE_EE_LE, &*o2, &e1, 2 },
    { Event::COMPARE_EE_EQ, &e1, &*o1, 2 },
    { Event::COMPARE_EE_NE, &e1, &*o1, 2 },
    { Event::COMPARE_EE_LT, &e1, &*o1, 2 },
    { Event::COMPARE_EE_GE, &e1, &*o1, 2 },
    { Event::COMPARE_EE_GT, &e1, &*o1, 2 },
    { Event::COMPARE_EE_LE, &e1, &*o1, 2 },
    { Event::COMPARE_EE_EQ, &e1, &*o2, 2 },
    { Event::COMPARE_EE_NE, &e1, &*o2, 2 },
    { Event::COMPARE_EE_LT, &e1, &*o2, 2 },
    { Event::COMPARE_EE_GE, &e1, &*o2, 2 },
    { Event::COMPARE_EE_GT, &e1, &*o2, 2 },
    { Event::COMPARE_EE_LE, &e1, &*o2, 2 },
    { Event::DESTRUCT, &e1, nullptr, 2 },
    { Event::DESTRUCT, &*o3, nullptr, 2 },
    { Event::DESTRUCT, &*o2, nullptr, 2 },
    { Event::DESTRUCT, &*o1, nullptr, 1 },
  };
  return true;
}

static bool testSwap(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o1{ 4 };
  auto const* v1 = &*o1;
  cm::optional<EventLogger> o2{};

  o1.swap(o2);
  auto const* v2 = &*o2;

  ASSERT_TRUE(!o1.has_value());
  ASSERT_TRUE(o2.has_value());
  ASSERT_TRUE(o2.value().Value == 4);

  o1.swap(o2);

  ASSERT_TRUE(o1.has_value());
  ASSERT_TRUE(o1.value().Value == 4);
  ASSERT_TRUE(!o2.has_value());

  o2.emplace(5);
  o1.swap(o2);

  ASSERT_TRUE(o1.has_value());
  ASSERT_TRUE(o1.value().Value == 5);
  ASSERT_TRUE(o2.has_value());
  ASSERT_TRUE(o2.value().Value == 4);

  o1.reset();
  o2.reset();
  o1.swap(o2);

  ASSERT_TRUE(!o1.has_value());
  ASSERT_TRUE(!o2.has_value());

  expected = {
    { Event::VALUE_CONSTRUCT, v1, nullptr, 4 },
    { Event::MOVE_CONSTRUCT, v2, v1, 4 },
    { Event::DESTRUCT, v1, nullptr, 4 },
    { Event::MOVE_CONSTRUCT, v1, v2, 4 },
    { Event::DESTRUCT, v2, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, v2, nullptr, 5 },
    { Event::SWAP, v1, v2, 5 },
    { Event::DESTRUCT, v1, nullptr, 5 },
    { Event::DESTRUCT, v2, nullptr, 4 },
  };
  return true;
}

static bool testReset(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o{ 4 };
  auto const* v = &*o;

  o.reset();

  ASSERT_TRUE(!o.has_value());

  o.reset();

  expected = {
    { Event::VALUE_CONSTRUCT, v, nullptr, 4 },
    { Event::DESTRUCT, v, nullptr, 4 },
  };
  return true;
}

static bool testEmplace(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o{ 4 };

  o.emplace(5);
  o.reset();
  o.emplace();

  expected = {
    { Event::VALUE_CONSTRUCT, &*o, nullptr, 4 },
    { Event::DESTRUCT, &*o, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, &*o, nullptr, 5 },
    { Event::DESTRUCT, &*o, nullptr, 5 },
    { Event::DEFAULT_CONSTRUCT, &*o, nullptr, 0 },
    { Event::DESTRUCT, &*o, nullptr, 0 },
  };
  return true;
}

static bool testMakeOptional(std::vector<Event>& expected)
{
  EventLogger e{ 4 };
  cm::optional<EventLogger> o1 = cm::make_optional<EventLogger>(e);
  cm::optional<EventLogger> o2 = cm::make_optional<EventLogger>(5);

  expected = {
    { Event::VALUE_CONSTRUCT, &e, nullptr, 4 },
    { Event::COPY_CONSTRUCT, &*o1, &e, 4 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
    { Event::DESTRUCT, &e, nullptr, 4 },
  };
  return true;
}

static bool testMemoryRange(std::vector<Event>& expected)
{
  cm::optional<EventLogger> o{ 4 };

  auto* ostart = &o;
  auto* oend = ostart + 1;
  auto* estart = &o.value();
  auto* eend = estart + 1;

  ASSERT_TRUE(static_cast<void*>(estart) >= static_cast<void*>(ostart) &&
              static_cast<void*>(eend) <= static_cast<void*>(oend));

  expected = {
    { Event::VALUE_CONSTRUCT, &*o, nullptr, 4 },
    { Event::DESTRUCT, &*o, nullptr, 4 },
  };
  return true;
}

int testOptional(int /*unused*/, char* /*unused*/[])
{
  int retval = 0;

#define DO_EVENT_TEST(name)                                                   \
  do {                                                                        \
    events.clear();                                                           \
    std::vector<Event> expected;                                              \
    if (!name(expected)) {                                                    \
      std::cout << "in " #name << std::endl;                                  \
      retval = 1;                                                             \
    } else if (expected != events) {                                          \
      std::cout << #name " did not produce expected events" << std::endl;     \
      retval = 1;                                                             \
    }                                                                         \
  } while (0)

#define DO_TEST(name)                                                         \
  do {                                                                        \
    if (!name()) {                                                            \
      std::cout << "in " #name << std::endl;                                  \
      retval = 1;                                                             \
    }                                                                         \
  } while (0)

  DO_EVENT_TEST(testDefaultConstruct);
  DO_EVENT_TEST(testNulloptConstruct);
  DO_EVENT_TEST(testValueConstruct);
  DO_EVENT_TEST(testInPlaceConstruct);
  DO_EVENT_TEST(testCopyConstruct);
  DO_EVENT_TEST(testMoveConstruct);
  DO_EVENT_TEST(testNulloptAssign);
  DO_EVENT_TEST(testCopyAssign);
  DO_EVENT_TEST(testMoveAssign);
  DO_EVENT_TEST(testPointer);
  DO_EVENT_TEST(testDereference);
  DO_EVENT_TEST(testHasValue);
  DO_EVENT_TEST(testValue);
  DO_TEST(testValueOr);
  DO_EVENT_TEST(testComparison);
  DO_EVENT_TEST(testSwap);
  DO_EVENT_TEST(testReset);
  DO_EVENT_TEST(testEmplace);
  DO_EVENT_TEST(testMakeOptional);
  DO_EVENT_TEST(testMemoryRange);

  return retval;
}
