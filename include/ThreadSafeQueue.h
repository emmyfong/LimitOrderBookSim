#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool finished_ = false;

public:
    //producer thread uses push to safely add item
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(item);
        lock.unlock();

        //wake up consumer
        cond_.notify_one(); 
    }

    //consumer uses pop to safely extract item
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        //sleep until queue is not empty or when producer says finished
        cond_.wait(lock, [this]() { return !queue_.empty() || finished_; });

        if (queue_.empty() && finished_) {
            //shut down
            return false;
        }

        item = queue_.front();
        queue_.pop();
        return true;
    }

    //producer calls setFinished when end of csv
    void setFinished() {
        std::unique_lock<std::mutex> lock(mutex_);
        finished_ = true;
        lock.unlock();
        cond_.notify_all();
    }

};