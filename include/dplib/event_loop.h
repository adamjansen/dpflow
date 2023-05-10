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
 * @file event_loop.h
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <chrono>

/**
 * @brief Event Loop
 */
class EventLoop
{
  public:
    using callable_t = std::function<void()>;

    EventLoop() = default;
    EventLoop(const EventLoop &) = delete;
    EventLoop(EventLoop &&) noexcept = delete;
    ~EventLoop() noexcept
    {
        if (!m_running)
            enqueue([this] { m_running = false; });

        if (m_thread.joinable())
            m_thread.join();
    }

    EventLoop &operator=(const EventLoop &) = delete;
    EventLoop &operator=(EventLoop &&) noexcept = delete;

    /**
     * @brief Stop the event loop
     */
    void stop()
    {
        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    /**
     * Get default event loop
     *
     * @return Default event loop
     */
    static std::shared_ptr<EventLoop> getDefault()
    {
        if (_default == nullptr) {
            _default = std::make_shared<EventLoop>();
        }
        return _default;
    }

    /**
     * @brief Run a function in teh event loop and wait for the result
     *
     * Adds a function to be executed in the event loop, and blocks until
     * the function is complete.
     *
     * @param[in] callable Function to call
     * @param[in] args Arguments to pass to function
     *
     * @return value returned from function
     */
    template <typename Func, typename... Args> auto enqueueSync(Func &&callable, Args &&...args)
    {
        if (std::this_thread::get_id() == m_thread.get_id()) {
            return std::invoke(std::forward<Func>(callable), std::forward<Args>(args)...);
        }

        using return_type = std::invoke_result_t<Func, Args...>;
        using packaged_task_type = std::packaged_task<return_type(Args && ...)>;

        packaged_task_type task(std::forward<Func>(callable));

        enqueue([&] { task(std::forward<Args>(args)...); });

        return task.get_future().get();
    }

    /**
     * @brief Add a function to be executed in the event loop
     *
     * Adds a function to be executed in the event loop, but
     * does not wait for the result.
     *
     * @param[in] callable Function to call
     * @param[in] args Arguments to pass to function
     *
     * @return the tasks's std::future
     */
    template <typename Func, typename... Args> [[nodiscard]] auto enqueueAsync(Func &&callable, Args &&...args)
    {
        using return_type = std::invoke_result_t<Func, Args...>;
        using packaged_task_type = std::packaged_task<return_type()>;

        auto task =
            std::make_shared<packaged_task_type>(std::bind(std::forward<Func>(callable), std::forward<Args>(args)...));

        enqueue(std::bind(&packaged_task_type::operator(), task));
        return task->get_future();
    }

    /**
     * @brief Execute function in event loop
     *
     * @param[in] callable Function to execute
     */
    void enqueue(callable_t &&callable) noexcept
    {
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_writeBuffer.emplace_back(std::move(callable));
        }
        m_condVar.notify_one();
    }

    /**
     * @brief Process all pending events
     *
     * All pending events will be processed until the timeout
     * is reached.  A @p timeout of 0 disables the timeout
     * and processes events forever.
     *
     * @param[in] timeoutMs Maximum time to process events
     *
     * @return true if one or more events were handled
     */
    bool processEvents(int timeoutMs = 0)
    {
        bool handled = false;
        std::vector<callable_t> readBuffer;

        // TODO: should the lock time be included in the timeout?
        auto start = std::chrono::steady_clock::now();

        // lock the buffer and grab all pending events
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condVar.wait(lock, [this] { return !m_writeBuffer.empty(); });
            std::swap(readBuffer, m_writeBuffer);
        }

        for (callable_t &func : readBuffer) {
            func();
            handled = true;
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (elapsed > timeoutMs) {
                break;
            }
        }

        readBuffer.clear();

        return handled;
    }

  private:
    std::vector<callable_t> m_writeBuffer;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
    bool m_running{true};
    std::thread m_thread{&EventLoop::threadFunc, this};

    static std::shared_ptr<EventLoop> _default;

    void threadFunc() noexcept
    {
        while (m_running) { processEvents(); }
    }
};
