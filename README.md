# thread_pool_high_performance

![thread_pool_c11](https://socialify.git.ci/578223592/thread_pool_c11/image?description=1&font=Source%20Code%20Pro&language=1&name=1&pattern=Circuit%20Board&theme=Auto)

本项目的目的是从0开始使用一些高级的技术实现一个高性能的线程池，并对比加入不同的技术之后其性能表现，用以作为其他项目的底层支撑

本项目最终的线程池支持的功能：

- 任务调度优化：比常规的线程池调度速度更快
- 支持优先级任务：支持按不同的优先级执行任务
- 线程池数量的动态调整：
- 长时任务的特殊分配，避免阻塞短时任务

## doc

- [线程池用到的新特性](./docs/涉及的新特性总结.md)

## 文件夹内容说明

todo：待完成后进行说明，如果看到这条可以issue提醒我

```
├── src
│ ├── version_1_inception 原始版本
│ │ └── include
│ └── version_2_high_performace 根据线程池手写版本，侧重运行逻辑的优化
│ └── include
```

## 如何测试
在tests文件夹中，随着项目进行进行补充


## 哪里可以使用？
理论上使用到多线程技术的项目都可以拿这个作为一个底层的支撑项目
### webServer

本项目主要用于webserver的优化:处理异步的一些无关紧要但是耗时的任务,这里也是模仿的redis：

- 关闭文件
- Redis中是unlink，可以使用线程池来管理LFU的缓存部分释放内存，将free任务交给线程池异步处理
- 其他可以异步执行的程序
