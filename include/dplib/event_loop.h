#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <chrono>

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

    void stop()
    {
        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    static std::shared_ptr<EventLoop> getDefault() {
        if (_default == nullptr) {
            _default = std::make_shared<EventLoop>();
        }
        return _default;
    }

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

    template <typename Func, typename... Args> [[nodiscard]] auto enqueueAsync(Func &&callable, Args &&...args)
    {
        using return_type = std::invoke_result_t<Func, Args...>;
        using packaged_task_type = std::packaged_task<return_type()>;

        auto task =
            std::make_shared<packaged_task_type>(std::bind(std::forward<Func>(callable), std::forward<Args>(args)...));

        enqueue(std::bind(&packaged_task_type::operator(), task));
        return task->get_future();
    }

    void enqueue(callable_t &&callable) noexcept
    {
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_writeBuffer.emplace_back(std::move(callable));
        }
        m_condVar.notify_one();
    }

    bool processEvents(int timeoutMs = 0) {
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
        while (m_running) {
            processEvents();
        }
    }
};
