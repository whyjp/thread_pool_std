#include "threadpool_std.h"

namespace threadpool_std {
ThreadPool::ThreadPool(size_t num_threads)
    : num_threads_(num_threads), stop_all(false)
{
    worker_threads_.reserve(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i) {
        worker_threads_.emplace_back([this]() { this->WorkerThread(); });
    }
}

void ThreadPool::WorkerThread()
{
    while (true) {
        std::unique_lock<std::mutex> lock(m_job_q_);
        cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
        if (stop_all && this->jobs_.empty()) {
            return;
        }

        // �� ���� job �� ����.
        std::function<void()> job = std::move(jobs_.front());
        jobs_.pop();
        lock.unlock();

        // �ش� job �� �����Ѵ� :)
        job();
    }
}

ThreadPool::~ThreadPool()
{
    stop_all = true;
    cv_job_q_.notify_all();

    for (auto& t : worker_threads_) {
        t.join();
    }
}

void ThreadPool::EnqueueJob(std::function<void()> job) {
    if (stop_all) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_job_q_);
        jobs_.push(std::move(job));
    }
    cv_job_q_.notify_one();
}

template <class F, class... Args>
std::future<typename std::invoke_result<F(Args...)>::type> ThreadPool::EnqueueJob(F&& f, Args&&... args)
{
    if (stop_all) {
        return;
    }

    using return_type = typename std::invoke_result<F(Args...)>::type;
    auto job = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> job_result_future = job->get_future();
    {
        std::lock_guard<std::mutex> lock(m_job_q_);
        jobs_.push([job]() { (*job)(); });
    }
    cv_job_q_.notify_one();

    return job_result_future;
}
}