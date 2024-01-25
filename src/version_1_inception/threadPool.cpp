//
// Created by swx on 23-8-10.
//

#include "threadPool.h"

namespace version_1_inception {
    ThreadPool::ThreadWorker::ThreadWorker(int id, ThreadPool *ofPool) : id_(id), pool_(ofPool) {
    }

    //拿到锁，然后陷入沉睡,等待唤醒
    void ThreadPool::ThreadWorker::operator()() {
        std::function<void()> thisTask{}; //无论是什么任务，在工作者看起来都是无参数无返回值的，因为返回值是通过packaged_task封装，又future来获取返回值的
        while (!pool_->stop_) {
            bool dequeued{}; {
                std::unique_lock<std::mutex> lock1(pool_->m_taskQueueconditionVariableMutex);
                if (pool_->m_taskSafeQueue.size() == 0) {
                    pool_->m_idleWorkerNum++;
                    pool_->m_taskQueueconditionVariable.wait(lock1); //使用mutex和条件变量的使用套路，要记住
                    pool_->m_idleWorkerNum--;
                }
                //出来之后可能是能取出，可能不能取出（比如已经停止了），要判断
                dequeued = pool_->m_taskSafeQueue.dequeue(thisTask);
            }
            if (dequeued) {
                thisTask();
                //todo ： 如果每次唤醒造成开销很大，尝试使用workbranch.h#L83 ，workbranch项目中
            }
            //执行完任务之后尝试唤醒
            pool_->m_taskDoneConditionVariable.notify_one();
        }
    }

    // 1.stop赋值为false 2.创建工作线程
    ThreadPool::ThreadPool(int threadsNum) : stop_(false), m_idleWorkerNum(0) {
        for (int i = 0; i < threadsNum; ++i) {
            workerVt_.emplace_back(ThreadWorker(i, this));
        }
    }

    // 这个的设计待考虑
    ThreadPool::~ThreadPool() {
        if (stop_ == false) {
            shutdown();
        }
    }


    void ThreadPool::shutdown() {
        stop_ = true;
        m_taskQueueconditionVariable.notify_all();
        for (int i = 0; i < workerVt_.size(); ++i) {
            if (workerVt_.at(i).joinable()) //如果还在执行，则等待当前执行的任务结束，还没有执行的就不管
            {
                workerVt_.at(i).join();
            }
        }
    }

    bool ThreadPool::waitTasksDone(const int32_t timeout_ms) {
        std::chrono::milliseconds waitTime{};
        if (timeout_ms == -1) {
            waitTime = std::chrono::milliseconds::max();
        } else {
            waitTime = std::chrono::milliseconds(timeout_ms);
        }
        bool res{}; {
            std::unique_lock<std::mutex> lock(m_taskQueueconditionVariableMutex);
            res = m_taskDoneConditionVariable.wait_for(lock, waitTime, [this]() {
                return workerVt_.size() == m_idleWorkerNum;
            });
        }

        return res;
    }
}
