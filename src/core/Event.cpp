#include <dplib/core/Event.h>

#include <mutex>
#include <set>

using namespace datapanel::core;

Event::Event(EventType type) : m_type(type), m_accept(true), m_posted(false)
{
}

Event::~Event()
{
    // TODO: remove references to pending posted event
}

class UserEventRegistry
{
  public:
    std::mutex mutex;
    std::set<int> set;
};

static UserEventRegistry userEventRegistry{};

int Event::registerEventType(int hint)
{
    std::lock_guard<std::mutex> guard(userEventRegistry.mutex);
    if (hint >= static_cast<int>(Event::EventType::User) && hint <= static_cast<int>(Event::EventType::MaxUser) &&
        !userEventRegistry.set.count(hint)) {
        userEventRegistry.set.insert(hint);
        return hint;
    }
    int id = static_cast<int>(Event::EventType::MaxUser);
    // look backwards to find an available user event id
    while (userEventRegistry.set.count(id) && id >= static_cast<int>(Event::EventType::User)) --id;

    if (id >= static_cast<int>(Event::EventType::User)) {
        userEventRegistry.set.insert(id);
        return id;
    }
    return -1;
}

TimerEvent::TimerEvent(int timerId) : Event(EventType::Timer), m_id(timerId)
{
}

TimerEvent::~TimerEvent()
{
}
