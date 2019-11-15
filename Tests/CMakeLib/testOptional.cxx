#include <iostream>
#include <type_traits>
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

void swap(EventLogger& e1, EventLogger& e2)
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

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::MOVE_CONSTRUCT, &*o2, &*o1, 4 },
    { Event::DESTRUCT, &*o2, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
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

  expected = {
    { Event::VALUE_CONSTRUCT, v2, nullptr, 4 },
    { Event::COPY_CONSTRUCT, v1, v2, 4 },
    { Event::VALUE_CONSTRUCT, v3, nullptr, 5 },
    { Event::COPY_ASSIGN, v1, v3, 5 },
    { Event::DESTRUCT, v1, nullptr, 5 },
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
  bool retval = true;

  const cm::optional<EventLogger> o1{ 4 };
  const cm::optional<EventLogger> o2{};

  if (!o1.has_value()) {
    std::cout << "o1 should have a value" << std::endl;
    retval = false;
  }

  if (!o1) {
    std::cout << "(bool)o1 should be true" << std::endl;
    retval = false;
  }

  if (o2.has_value()) {
    std::cout << "o2 should not have a value" << std::endl;
    retval = false;
  }

  if (o2) {
    std::cout << "(bool)o2 should be false" << std::endl;
    retval = false;
  }

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return retval;
}

static bool testValue(std::vector<Event>& expected)
{
  bool retval = true;

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
  if (!thrown) {
    std::cout << "o3.value() did not throw" << std::endl;
    retval = false;
  }

  thrown = false;
  try {
    (void)o4.value();
  } catch (cm::bad_optional_access&) {
    thrown = true;
  }
  if (!thrown) {
    std::cout << "o4.value() did not throw" << std::endl;
    retval = false;
  }

  expected = {
    { Event::VALUE_CONSTRUCT, &*o1, nullptr, 4 },
    { Event::VALUE_CONSTRUCT, &*o2, nullptr, 5 },
    { Event::REFERENCE, &*o1, nullptr, 4 },
    { Event::CONST_REFERENCE, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o2, nullptr, 5 },
    { Event::DESTRUCT, &*o1, nullptr, 4 },
  };
  return retval;
}

static bool testValueOr()
{
  bool retval = true;

  const cm::optional<EventLogger> o1{ 4 };
  cm::optional<EventLogger> o2{ 5 };
  const cm::optional<EventLogger> o3{};
  cm::optional<EventLogger> o4{};

  EventLogger e1{ 6 };
  EventLogger e2{ 7 };
  EventLogger e3{ 8 };
  EventLogger e4{ 9 };

  EventLogger r1 = o1.value_or(e1);
  if (r1.Value != 4) {
    std::cout << "r1.Value should be 4" << std::endl;
    retval = false;
  }
  EventLogger r2 = std::move(o2).value_or(e2);
  if (r2.Value != 5) {
    std::cout << "r2.Value should be 5" << std::endl;
    retval = false;
  }
  EventLogger r3 = o3.value_or(e3);
  if (r3.Value != 8) {
    std::cout << "r3.Value should be 8" << std::endl;
    retval = false;
  }
  EventLogger r4 = std::move(o4).value_or(e4);
  if (r4.Value != 9) {
    std::cout << "r4.Value should be 9" << std::endl;
    retval = false;
  }

  return retval;
}

static bool testSwap(std::vector<Event>& expected)
{
  bool retval = true;

  cm::optional<EventLogger> o1{ 4 };
  auto const* v1 = &*o1;
  cm::optional<EventLogger> o2{};

  o1.swap(o2);
  auto const* v2 = &*o2;

  if (o1.has_value()) {
    std::cout << "o1 should not have value" << std::endl;
    retval = false;
  }
  if (!o2.has_value()) {
    std::cout << "o2 should have value" << std::endl;
    retval = false;
  }
  if (o2.value().Value != 4) {
    std::cout << "value of o2 should be 4" << std::endl;
    retval = false;
  }

  o1.swap(o2);

  if (!o1.has_value()) {
    std::cout << "o1 should have value" << std::endl;
    retval = false;
  }
  if (o1.value().Value != 4) {
    std::cout << "value of o1 should be 4" << std::endl;
    retval = false;
  }
  if (o2.has_value()) {
    std::cout << "o2 should not have value" << std::endl;
    retval = false;
  }

  o2.emplace(5);
  o1.swap(o2);

  if (!o1.has_value()) {
    std::cout << "o1 should have value" << std::endl;
    retval = false;
  }
  if (o1.value().Value != 5) {
    std::cout << "value of o1 should be 5" << std::endl;
    retval = false;
  }
  if (!o2.has_value()) {
    std::cout << "o2 should not have value" << std::endl;
    retval = false;
  }
  if (o2.value().Value != 4) {
    std::cout << "value of o2 should be 4" << std::endl;
    retval = false;
  }

  o1.reset();
  o2.reset();
  o1.swap(o2);

  if (o1.has_value()) {
    std::cout << "o1 should not have value" << std::endl;
    retval = false;
  }
  if (o2.has_value()) {
    std::cout << "o2 should not have value" << std::endl;
    retval = false;
  }

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
  return retval;
}

static bool testReset(std::vector<Event>& expected)
{
  bool retval = true;

  cm::optional<EventLogger> o{ 4 };
  auto const* v = &*o;

  o.reset();

  if (o.has_value()) {
    std::cout << "o should not have value" << std::endl;
    retval = false;
  }

  o.reset();

  expected = {
    { Event::VALUE_CONSTRUCT, v, nullptr, 4 },
    { Event::DESTRUCT, v, nullptr, 4 },
  };
  return retval;
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
  bool retval = true;

  cm::optional<EventLogger> o{ 4 };

  auto* ostart = &o;
  auto* oend = ostart + 1;
  auto* estart = &o.value();
  auto* eend = estart + 1;

  if (static_cast<void*>(estart) < static_cast<void*>(ostart) ||
      static_cast<void*>(eend) > static_cast<void*>(oend)) {
    std::cout << "value is not within memory range of optional" << std::endl;
    retval = false;
  }

  expected = {
    { Event::VALUE_CONSTRUCT, &*o, nullptr, 4 },
    { Event::DESTRUCT, &*o, nullptr, 4 },
  };
  return retval;
}

int testOptional(int /*unused*/, char* /*unused*/ [])
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
  DO_EVENT_TEST(testSwap);
  DO_EVENT_TEST(testReset);
  DO_EVENT_TEST(testEmplace);
  DO_EVENT_TEST(testMakeOptional);
  DO_EVENT_TEST(testMemoryRange);

  return retval;
}
