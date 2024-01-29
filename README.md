# thread_pool_high_performance

本项目的目的是从0开始使用一些高级的技术实现一个高性能的线程池，并对比加入不同的技术之后其性能表现，用以作为其他项目的底层支撑

## doc

- [线程池用到的新特性](./docs/涉及的新特性总结.md)

## 文件夹内容说明

todo：待完成后进行说明，如果看到这条可以issue提醒我

├── src
│ ├── version_1_inception 原始版本
│ │ └── include
│ └── version_2_high_performace 根据线程池手写版本，侧重运行逻辑的优化
│ └── include

## 如何测试

c11通用的可以接受任意任务的异步线程池
本项目主要用于webserver的优化:处理异步的一些无关紧要但是耗时的任务,这里也是模仿的redis：

- 关闭文件
- Redis中是unlink，可以使用线程池来管理LFU的缓存部分释放内存，将free任务交给线程池异步处理
- 其他可以异步执行的程序
