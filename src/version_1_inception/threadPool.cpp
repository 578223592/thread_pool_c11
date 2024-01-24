//
// Created by swx on 23-8-10.
//

#include "threadPool.h"




ThreadPool::ThreadWorker::ThreadWorker(int id, ThreadPool *ofPool) : id_(id), pool_(ofPool){

}

//拿到锁，然后陷入沉睡,等待唤醒
void ThreadPool::ThreadWorker::operator()()
{
    function<void()> thisTask;
    while (!pool_->stop_){
        bool dequeued = false;
        {
            unique_lock<mutex> lock1(pool_->conditionVariableMutex_);
            if(pool_->safeQueue_.size()==0){
                pool_->conditionVariable_.wait(lock1);  //使用mutex和条件变量的使用套路，要记住
            }
            //出来之后可能是能取出，可能不能取出（比如已经停止了），要判断
            dequeued = pool_->safeQueue_.dequeue(thisTask);
        }
        if(dequeued){
            thisTask();
        }
    }
}

// 1.stop赋值为false 2.创建工作线程
ThreadPool::ThreadPool(int threadsNum) : stop_(false)
{
    for (int i = 0; i < threadsNum; ++i)
    {
        workerVt_.emplace_back(thread(ThreadWorker(i, this)));
    }
}
// 这个的设计待考虑
ThreadPool::~ThreadPool()
{
}


void ThreadPool::shutdown()
{
    stop_ = true;
    conditionVariable_.notify_all();
    for (int i = 0; i < workerVt_.size(); ++i)
    {
        if (workerVt_.at(i).joinable())  //如果还在执行，则等待当前执行的任务结束，还没有执行的就不管
        {
            workerVt_.at(i).join();
        }
    }
}