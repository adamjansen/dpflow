#include <dplib/core/EventDispatcher.h>
#include <dplib/core/Timer.h>
#include <spdlog/spdlog.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <atomic>

using namespace datapanel::core;

static std::atomic_int nextTimerId{1};

EventDispatcher::EventDispatcher() :m_interrupt(false) {
    m_epoll_fd = ::epoll_create1(0);
}

EventDispatcher::~EventDispatcher() {
    ::close(m_epoll_fd);
}

int EventDispatcher::addTimer(int periodMs, std::function<void()> func) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int id = nextTimerId++;
    m_timers.emplace_back(id, periodMs, func);
    m_timers.sort();

    return id;
}

bool EventDispatcher::removeTimer(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int countBefore = m_timers.size();

    m_timers.remove_if([&](const TimerInfo& t) { return t.id == id; });
    int countAfter = m_timers.size();
    return (countAfter - countBefore) > 0;
}

bool EventDispatcher::addFile(int fd, FileOperation op, FileFunc func)
{
    if (fd < 0)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    // Prevent duplicates
    std::pair<int, FileOperation> key{fd, op};
    if (m_files.count(key) > 0)
        return false;

    struct epoll_event event;
    event.data.fd = fd;
    switch (op) {
        case Read:
            event.events = EPOLLIN;
            break;
        case Write:
            event.events = EPOLLOUT;
            break;
        case Error:
            event.events = EPOLLERR;
            break;
        default:
            // TODO: log error?
            return false;
    }

    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event)) {
        return false;
    }

    m_files[key] = func;

    return true;
}

bool EventDispatcher::removeFile(int fd, FileOperation op)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::pair<int, FileOperation> key{fd, op};
    if (m_files.count(key) == 0)
        return false;

    struct epoll_event event;
    event.data.fd = fd;
    switch (op) {
        case Read:
            event.events = EPOLLIN;
            break;
        case Write:
            event.events = EPOLLOUT;
            break;
        case Error:
            event.events = EPOLLERR;
            break;
        default:
            // TODO: log error?
            return false;
    }

    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &event)) {
        return false;
    }

    m_files.erase(key);

    return true;
}

bool EventDispatcher::pendingEvents() {
    return false;
}

bool EventDispatcher::processEvents() {
    m_interrupt = false;

    int count = 0;

    int timeoutMs = -1;
    if (m_timers.size() > 0) {
        auto earliestTimer = m_timers.begin();
        auto remaining = earliestTimer->expiry - std::chrono::steady_clock::now();
        timeoutMs = std::chrono::ceil<std::chrono::milliseconds>(remaining).count();
    }

    constexpr int maxEvents = 10;
    struct epoll_event events[maxEvents];
    int readyFds = ::epoll_wait(m_epoll_fd, events, maxEvents, timeoutMs);
    for (int n = 0; n < readyFds; n++) {
        int fd = events[n].events;
        FileOperation op = static_cast<FileOperation>(events[n].data.fd);
        std::pair<int,FileOperation> key{fd, op};
        if (m_files.count(key) > 0) {
            m_files[key]();
        } else {
            // TODO: log error for unexpected event?
        }
    }

    for (auto& t : m_timers) {
        if (t.expired()) {
            t.fire();
            count++;
        } else {
            // It's a sorted list, so we can stop now
            break;
        }
    }

    return count > 0;
}
