/**
 * Copyright (c) 2023 Data Panel Corporation
 * 181 Cheshire Ln, Suite 300
 * Plymouth, MN 55441
 * All rights reserved.
 *
 * This is the confidential and proprietary information of Data Panel
 * Corporation. Such confidential information shall not be disclosed and is for
 * use only in accordance with the license agreement you entered into with Data
 * Panel.
 */

/**
 * @file Event.h
 * @author ajansen
 * @date 2023-05-10
 */

#pragma once

namespace datapanel
{
namespace core
{
class Event {
public:
    enum class EventType {
        None = 0,
        Timer = 1,
        Quit = 100,
        User = 10000,
        MaxUser = 65535,
    };

    Event(EventType type);
    virtual ~Event();

    inline EventType type() const { return m_type;}

    inline void accept() { m_accept = true; }
    inline void reject() { m_accept = false;}

    static int registerEventType(int hint = -1);
protected:
    EventType m_type;

private:
    bool m_accept;
    bool m_posted;

    friend class Application;

};

class TimerEvent : public Event {
public:
    TimerEvent(int timerId);
    ~TimerEvent();
    int timerId() const  {return m_id;}

protected:
    int m_id;
};

}
}
