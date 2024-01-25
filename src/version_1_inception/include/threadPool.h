//
// Created by swx on 23-8-10.
//

#ifndef THREAD_POOL_C11_THREADPOOL_H
#define THREAD_POOL_C11_THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <bitset> //STL位集容器
#include <cctype>
#include <cerrno>
#include <clocale>
#include <cmath>
#include <complex> //复数类
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>     //STL双端队列容器
#include <exception> //异常处理类
#include <fstream>
#include <functional> //STL定义运算函数（代替运算符）
#include <limits>
#include <list> //STL线性列表容器
#include <map>  //STL 映射容器
#include <iomanip>
#include <ios>    //基本输入／输出支持
#include <iosfwd> //输入／输出系统使用的前置声明
#include <iostream>
#include <istream>   //基本输入流
#include <ostream>   //基本输出流
#include <queue>     //STL队列容器
#include <set>       //STL 集合容器
#include <sstream>   //基于字符串的流
#include <stack>     //STL堆栈容器　　　　
#include <stdexcept> //标准异常类
#include <streambuf> //底层输入／输出支持
#include <string>    //字符串类
#include <utility>   //STL通用模板类
#include <vector>    //STL动态数组容器
#include <cwchar>
#include <cwctype>
#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include <bits/stdc++.h>
#include <functional>
using namespace std;

// Definition for a Node.
class Node
{
public:
    int val;
    Node *left;
    Node *right;
    Node *next;

    Node() : val(0), left(nullptr), right(nullptr), next(nullptr) {}

    explicit Node(int _val) : val(_val), left(nullptr), right(nullptr), next(nullptr) {}

    Node(int _val, Node *_left, Node *_right, Node *_next)
            : val(_val), left(_left), right(_right), next(_next) {}
};

//* Definition for a binary tree node.
struct TreeNode
{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

// Definition for singly-linked list.
struct ListNode
{
    int val;
    ListNode *next;
    explicit ListNode(int x) : val(x), next(nullptr) {}
};

// 积木1：任务队列
// 负责存在任务，即 function<void()>
// 对外暴露三个接口：size enqueue dequeue
template <typename T>
class ThreadSafeDeque
{
private:
    queue<T> queue_;
    mutex mutex_;

public:
    ThreadSafeDeque(/* args */);
    ~ThreadSafeDeque();

public:
    size_t size();
    void enqueue(T &t);
    bool dequeue(T &t);
};
template <typename T>
ThreadSafeDeque<T>::ThreadSafeDeque(/* args */)
= default;

template <typename T>
ThreadSafeDeque<T>::~ThreadSafeDeque()
= default;

template <typename T>
size_t ThreadSafeDeque<T>::size()
{
    lock_guard<mutex> lock1(mutex_);
    return queue_.size();
}

template <typename T>
void ThreadSafeDeque<T>::enqueue(T &t)
{
    lock_guard<mutex> lock1(mutex_);
    queue_.push(move(t));
}

template <typename T>
bool ThreadSafeDeque<T>::dequeue(T &t)
{
    lock_guard<mutex> lock1(mutex_);
    if (queue_.size() == 0)
    {
        return false;
    }
    t = std::move(queue_.front()); // 取出队首元素，返回队首元素值，并进行右值引用
    queue_.pop();
    return true;
}


// 线程池
class ThreadPool
{
private:

// 积木2：工作类
//  正常写，只需要注意：1.拿到pool的指针 2.只需要重载()操作符即可，即一直执行即可
// 实际上只是一个仿函数，为了添加进thread使用
    class ThreadWorker
    {
    private:
        int id_;
        ThreadPool *pool_; // 要返过去拿到线程池才能拿到其条件变量和锁
    public:
        ThreadWorker(int id, ThreadPool *ofPool);
        ~ThreadWorker() = default;
        void operator()(); // 重载需要完成的事情，循环去拿任务，然后完成，任务是function<void()>
    };



public:
    condition_variable conditionVariable_;
    mutex conditionVariableMutex_; // 这个锁的目的是和条件变量配合唤醒，任务队列已经是安全的了，主要是给子线程用，不是当前使用的
    bool stop_;
    vector<thread> workerVt_;                     // 注意保存是线程而不是worker本身
    ThreadSafeDeque<function<void()>> safeQueue_; // 任务存放的都是function<void()> 类型
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
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> future<decltype(f(args...))>; // 最难得函数，语法糖是最多的
};



//大概步骤：先通过bind绑定成一个function，然后通过packaged_task得到保存packaged_task的shared_ptr
//要shared_ptr的原因是这有两处要使用：1.加入任务队列；2.返回future
// 上面返回future是通过packaged_task来得到的，得到future也是使用packaged_task的目的。
template <typename F, typename... Args>
auto ThreadPool::submit(F &&f, Args &&...args) -> future<decltype(f(args...))> {  //注意！！写法上注意这几个...的位置，含义不同位置就不同
    function<decltype(f(args...))()> func = bind(forward<F>(f),forward<Args>(args)...);
    auto task_ptr = make_shared<packaged_task<decltype(f(args...))()>>(func);  //这里和上面decltype都是两个括号，想清楚，因为funciton括号外是返回值，括号内是参数

    function<void()> wrapper_func = [task_ptr]() {   //如果这里不是make_shared，那么就相当于是复制了一个packaged_task，值拷贝。
        (*task_ptr)();
    };

    safeQueue_.enqueue(wrapper_func);
    conditionVariable_.notify_one();

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



#endif //THREAD_POOL_C11_THREADPOOL_H
