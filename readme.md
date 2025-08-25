\# Footprints

纯C++的个人日记网站

http://119.45.10.196:8080/

编译运行

```bash
mkdir build && cd build && cmake .. && cmake --build . && ./footprints
```



结构说明：

`log`是一个独立线程的日志，有任务队列，线程安全。

`socket`是对操作系统套接字的简单封装。

`http`是在套接字的基础上的简单`http`协议解析和构建。

`router`是路由控制，按目录的格式解析，直到有注册的路径

`handler`是要定义的处理函数，注册在路由上，访问的时候调用。

`assets`是静态资源，`html`、`js`等，页面可以做在里面。