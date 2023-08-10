# thread_pool_c11

todo:websever星球解读已经差不多看懂了，待再复习复习，手撕，总结！！！

预计要花个四五小时了。。。



> 主要参考文章：https://zhuanlan.zhihu.com/p/367309864
> 
> 这个博主还有些其他文章干活满满

学习线程池的过程中发现其中有很多c11的特性需要学习，并不简单

## 前置知识

```c++
template<typename F, typename... Args>
auto enqueue(F &&f, Args &&...args); // 入队接口
```



std::future 用于异步获取返回值
