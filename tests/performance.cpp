//
// Created by swx on 24-1-24.
//
#include "threadPool.h"
#include <atomic>
#include <nanobench.h>
// std::atomic<int> a = 0;

void benchmark1(const size_t taskNum, const size_t threadNum) {
    std::cout << "benchmark1 start" << std::endl;

    // std::cout << a << std::endl;
    version_1_inception::ThreadPool pool(threadNum);
    for (size_t i = 0; i < taskNum; ++i) {
        auto task = []() {
            std::cout << "hello world!" << "\n";
            // a.fetch_sub(1);
            return 1;
        };
        // a.fetch_add(1);
        auto future = pool.submit(task);
        // a.fetch_sub(future.get());
        // future.get();
    }
    // std::cout << a << std::endl;
    while (!pool.waitTasksDone()) {
        // std::cout << "---" << "\n";
    }
    //todo :有时候a不为0，原因未知，待检查
    // std::cout << "a3:" << a.load() << std::endl;
    std::cout << "benchmark1 finished" << std::endl;
}

int main() {
    benchmark1(100000, 20);

    return 0;
}
