//
// Created by swx on 23-8-10.
//

#ifndef THREAD_POOL_C11_THREADPOOL_H_version_1
#define THREAD_POOL_C11_THREADPOOL_H_version_1
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <functional> //STL定义运算函数（代替运算符）
#include <queue>     //STL队列容器
#include <utility>   //STL通用模板类
#include <vector>    //STL动态数组容器
#include <bits/stdc++.h>
#include <functional>


namespace version_1_inception {
    // Definition for a Node.
    class Node {
    public:
        int val;
        Node *left;
        Node *right;
        Node *next;

        Node() : val(0), left(nullptr), right(nullptr), next(nullptr) {
        }

        explicit Node(int _val) : val(_val), left(nullptr), right(nullptr), next(nullptr) {
        }

        Node(int _val, Node *_left, Node *_right, Node *_next)
            : val(_val), left(_left), right(_right), next(_next) {
        }
    };

    //* Definition for a binary tree node.
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;

        TreeNode() : val(0), left(nullptr), right(nullptr) {
        }

        explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {
        }

        TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {
        }
    };

    // Definition for singly-linked list.
    struct ListNode {
        int val;
        ListNode *next;

        explicit ListNode(int x) : val(x), next(nullptr) {
        }
    };

    // 积木1：任务队列
    // 负责存在任务，即 function<void()>
    // 对外暴露三个接口：size enqueue dequeue
    template<typename T>
    class ThreadSafeDeque {
    private:
        std::queue<T> queue_;
        std::mutex mutex_;

    public:
        ThreadSafeDeque(/* args */);

        ~ThreadSafeDeque();

    public:
        size_t size();

        void enqueue(T &t);

        void enqueue(T &&t);

        bool dequeue(T &t);
    };

    template<typename T>
    ThreadSafeDeque<T>::ThreadSafeDeque(/* args */)
    = default;

    template<typename T>
    ThreadSafeDeque<T>::~ThreadSafeDeque()
    = default;

    template<typename T>
    size_t ThreadSafeDeque<T>::size() {
        std::lock_guard<std::mutex> lock1(mutex_);
        return queue_.size();
    }

    template<typename T>
    void ThreadSafeDeque<T>::enqueue(T &t) {
        std::lock_guard<std::mutex> lock1(mutex_);
        queue_.push(std::move(t));
    }

    template<typename T>
    void ThreadSafeDeque<T>::enqueue(T &&t) {
        std::lock_guard<std::mutex> lock1(mutex_);
        queue_.push(std::move(t));
    }

    template<typename T>
    bool ThreadSafeDeque<T>::dequeue(T &t) {
        std::lock_guard<std::mutex> lock1(mutex_);
        if (queue_.empty()) {
            return false;
        }
        t = std::move(queue_.front()); // 取出队首元素，返回队首元素值，并进行右值引用
        queue_.pop();
        return true;
    }


    // 线程池
    class ThreadPool {
    private:
        // 积木2：工作类
        //  正常写，只需要注意：1.拿到pool的指针 2.只需要重载()操作符即可，即一直执行即可
        // 实际上只是一个仿函数，为了添加进thread使用
        class ThreadWorker {
        private:
            int id_;
            ThreadPool *pool_; // 要返过去拿到线程池才能拿到其条件变量和锁
        public:
            ThreadWorker(int id, ThreadPool *ofPool);

            ~ThreadWorker() = default;

            void operator()(); // 重载需要完成的事情，循环去拿任务，然后完成，任务是function<void()>
        };

    private:
        std::condition_variable m_taskDoneConditionVariable;
        std::condition_variable m_taskQueueconditionVariable;
        std::mutex m_taskQueueconditionVariableMutex; // 这个锁的目的是和条件变量配合唤醒，任务队列已经是安全的了，主要是给子线程用，不是当前使用的,保证子线程的唤醒与睡眠
        bool stop_;
        std::vector<std::thread> workerVt_; // 注意保存是线程而不是worker本身
        ThreadSafeDeque<std::function<void()> > m_taskSafeQueue; // 任务存放的都是function<void()> 类型
        /**
         * \brief 当前idle的工作者（线程）数量，idle即陷入条件变量的沉睡
         */
        size_t m_idleWorkerNum;

    public:
        // start 禁止拷贝
        ThreadPool(const ThreadPool &) = delete;

        ThreadPool(ThreadPool &&) = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        ThreadPool &operator=(ThreadPool &&) = delete;

        // end 禁止拷贝

        explicit ThreadPool(int threadsNum);

        ~ThreadPool();

    public:
        void shutdown();

        template<typename F, typename... Args>
        auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>; // 最难懂的函数，语法糖是最多的
        /**
         * \brief 等待所有任务完成
         * \param timeout_ms 最长超时时间，单位ms;-1表示无限等待；默认无限等待
         * \return 超时时间到达的时候，所有任务是否执行完毕
         * \note 实际判断的是idle的线程数量？=总线程数；队列为空！=任务执行完毕，因为还有可能还有些线程还在执行，因此需要成员变量来保存休息的线程数量
         */
        bool waitTasksDone(int32_t timeout_ms = -1);
    };


    //大概步骤：先通过bind绑定成一个function，然后通过packaged_task得到保存packaged_task的shared_ptr
    //要shared_ptr的原因是这有两处要使用：1.加入任务队列；2.返回future
    // 上面返回future是通过packaged_task来得到的，得到future也是使用packaged_task的目的。
    template<typename F, typename... Args>
    auto ThreadPool::submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        //注意！！写法上注意这几个...的位置，含义不同位置就不同
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        //notice:将任务打包成shared_ptr的原因在于任务是会丢进线程池异步执行的，即有概率离开当前函数后才执行，那么就要用shared_ptr再来包装一层
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()> >(func);
        //这里和上面decltype都是两个括号，想清楚，因为funciton括号外是返回值，括号内是参数
        std::function<void()> wrapper_func = [task_ptr]() {
            //如果这里不是make_shared，那么就相当于是复制了一个packaged_task，值拷贝。
            (*task_ptr)(); //执行封装的func
        };

        m_taskSafeQueue.enqueue(wrapper_func);
        m_taskQueueconditionVariable.notify_one();

        return task_ptr->get_future();
    }


    /**
    1.这里面泛型类为何只有threadSafeQueue
    答案：这里面一共有三个类，我们可以一个一个挨着分析，分别为：threadSafeQueue，worker，threadPool
        threadSafeQueue：因为是实现的任意类型的线程安全的队列，因此就使用的泛型，其实如果按照实现线程池来说的话，不用泛型，直接是function<void()>即可
        worker：肯定不用泛型，因为本质上是一个仿函数，给c11新特性thread使用的而已
        threadPool：也不用泛型，只用提交任务的时候（submit函数）支持泛型即可，而这个函数也是最重要最难的地方。

    对于线程安全queue实现的必要性：
    实现线程安全的queue的目的意义其实不是很大，我们可以看到在线程池类里面有条件变量配合互斥锁来保证 取出任务时候的并发安全。
    那么只有加入队列的时候可能需要线程不安全，那么在加入任务的时候再上一下锁其实就可以保证安全了。
    所以线程安全的queue实现的必要性不是那么强，不过实现了其实也可以，因为锁只有冲突的时候耗时才会很高，不冲突有fu mutex（快速用户锁）等方案。
    **/
}


#endif //THREAD_POOL_C11_THREADPOOL_H
