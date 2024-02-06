//
// Created by swx on 23-8-10.
//

#ifndef THREAD_POOL_C11_THREADPOOL_H_version_2
#define THREAD_POOL_C11_THREADPOOL_H_version_2
#include <functional>
#include <thread>
#include "threadPoolConfig.h"
#include "UThreadPrimary.h"

namespace version_2_high_performace {
    /**
     * \brief 线程池类
     */
    class thread_pool final {
    public:
        bool submit(std::function<void()> &func);

        bool init(const ThreadPoolConfig &config, bool autoInit = true);

        bool waitTaskDowned(int32_t timeout_ms = -1);

    private:
        ThreadPoolConfig config_;
        UAtomicQueue<UTask> taskQueue_;
        /**
         * \brief 线程队列
         */
        std::vector<UThreadPrimary *> primaryThreads_;
    };
}


#endif //THREAD_POOL_C11_THREADPOOL_H
