#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>

// Fixed-size pool of worker threads that pull tasks from a shared queue.
// Avoids the overhead of spawning/destroying a thread per client connection.
class ThreadPool {
public:
    ThreadPool(size_t n) {
        for (size_t i = 0; i < n; i++) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        // unique_lock required here — condition variable needs
                        // to temporarily release the lock while thread sleeps
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    // Add a task to the queue and wake one sleeping worker
    void enqueue(std::function<void()> task) {
        { std::lock_guard<std::mutex> lock(mtx); tasks.push(std::move(task)); }
        cv.notify_one();
    }

    // Signal shutdown, wake all threads, wait for them to finish
    ~ThreadPool() {
        { std::lock_guard<std::mutex> lock(mtx); stop = true; }
        cv.notify_all();
        for (auto& t : workers) t.join();
    }

private:
    std::vector<std::thread>          workers;
    std::queue<std::function<void()>> tasks;
    std::mutex                        mtx;
    std::condition_variable           cv;
    bool                              stop = false;
};