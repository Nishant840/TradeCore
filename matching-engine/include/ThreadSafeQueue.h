#pragma once

#include<queue>
#include<mutex>
#include<condition_variable>

template <typename T>

class ThreadSafeQueue {
  public:
    void push(T item){
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(item));
        cv.notify_one();
    }

    T waitAndPop(){
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] {
            return !queue.empty();
        });

        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    bool empty() const{
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

  private:
    std::queue<T>           queue;
    mutable std::mutex      mtx;
    std::condition_variable cv;
};