---
id: "2965506198"
title: "c++11如何实现单例模式？"
author: "pansz"
type: zhihu-answer
source: "https://www.zhihu.com/question/50533404/answer/2965506198"
downloaded: "2026-10-31"
---
如果你只是需要全局访问，把一堆函数跟变量直接放进某个新 namespace 中就行。连 class 都不需要定义。

namespace 就是 C++ 语言级天生的单例。你有没有想过单例根本不需要是 class？

之所以有单例模式这种东西，本质上是因为 Java 没有全局变量。

都写 C++ 了，就不必坚持用 class 来制造单例了。

除非你需要使用的是某个系统级，无法重复获取的东西。

* * *

补充：如果需要控制某个变量的初始化时机，怎么办？答案是把它作为这个namespace的私有变量，自行控制其初始化进程。然后增加相关函数用于主动触发相关初始化。