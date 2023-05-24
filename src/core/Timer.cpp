#include <dplib/core/Timer.h>

using namespace datapanel::core;

Timer::Timer() : m_id(0), m_period(0), m_oneshot(false)
{
}

Timer::~Timer()
{
    if (m_id >= 0)
        stop();
}

void Timer::start()
{
    if (m_id >= 0)
        stop();
    m_id = 1;
}

void Timer::start(int periodMs)
{
    m_period = periodMs;
    start();
}

void Timer::stop()
{
    if (m_id >= 0) {
        // stop timer
        m_id = 0;
    }
}

void Timer::setOneshot(bool oneshot) {
    m_oneshot = oneshot;
}

void Timer::setPeriod(int ms) {
    m_period = ms;
}
