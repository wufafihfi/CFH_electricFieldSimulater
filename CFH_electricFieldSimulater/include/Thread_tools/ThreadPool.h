#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

//###############学习#################
// 阅 - 2遍
//####################################

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        { std::unique_lock<std::mutex> lock(queueMutex); stop = true; }
        condition.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

    template<class F>
    auto enqueue(F&& task) -> std::future<void> {
        auto packagedTask = std::make_shared<std::packaged_task<void()>>(std::forward<F>(task));
        std::future<void> result = packagedTask->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace([packagedTask] { (*packagedTask)(); });
        }
        condition.notify_one();
        return result;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

/*
ThreadPool pool(4);

// 提交无参数任务
std::future<void> f = pool.enqueue([]() {
    // 做一些计算
});

// 等待完成
f.wait();



提交带参数任务

ThreadPool pool(4);

auto task = pool.enqueue([](int a, int b) {
    return a + b;
}, 1, 2);

*/