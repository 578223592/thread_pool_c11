//
// Created by swx on 24-1-24.
//
#include "threadPool1.h"
#include <atomic>
#include <nanobench.h>
// std::atomic<int> a = 0;

void benchmark1(ankerl::nanobench::Bench *bench, const std::string &benchName, const size_t taskNum,
                const size_t threadNum) {
    version_1_inception::ThreadPool pool(threadNum);
    bench->run(benchName, [&]()-> void {
        // std::cout << "benchmark1 start" << std::endl;

        // std::cout << a << std::endl;
        for (size_t i = 0; i < taskNum; ++i) {
            auto task = []() {
                // std::cout << "hello world!" << "\n";
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
        // std::cout << "benchmark1 finished" << std::endl;
    });
    pool.shutdown();
}

int main() {
    //todo :看一下相关参数设置的含义，目前是看的bench1.cc#L7 workspace项目

    ankerl::nanobench::Bench bench;
    bench.relative(true);
    bench.performanceCounters(true);
    bench.output(&std::cout);
    bench.timeUnit(std::chrono::milliseconds{1}, "ms");
    bench.minEpochIterations(10);
    const int taskNum = 10000, threadNum = 2;
    std::string name = "taskNum:" + std::to_string(taskNum) + " threadNum:" + std::to_string(threadNum);
    benchmark1(&bench, name + "111", taskNum, threadNum);
    benchmark1(&bench, name + "222", taskNum, threadNum);
    benchmark1(&bench, name + "333", taskNum, threadNum);


    std::cout << "BENCH END";
    return 0;
}
