# raycast2d-draw-server

**业余练手**  

websocket处理已通用，通信协议采用protobuf。若更换业务场景，需重写proto文件，并注册于worker.h。
## 参考
- [Raycasting](https://lodev.org/cgtutor/raycasting.html#Performance)
- [raycast-2d](https://github.com/moumoulongs/raycast-2d)

## 文件说明
```markdown
    ...
    ├── cuda
    │   └── main.cu                        // GPU计算入口
    ├── include
    │   ├── camera.h                       // 屏幕绘制逻辑
    │   ├── client.h                       // uWebsockets客户端封装
    │   ├── infrastructure.h               // 基础设施
    │   ├── map2d.h                        // 地图
    │   ├── mredis.h                       // Redis-API
    │   ├── room.h                         // 玩家房间
    │   ├── user.h                         // 客户端连接信息
    │   └── worker.h                       // uWebSockets服务端封装
    ├── src
    │   └── main.cc                        // 主控制逻辑
    ├── client.py                          // python客户端
    ...
```

## 运行环境
```markdown
硬件环境:
    - Ubuntu 20.04.6 LTS
    - Kernal 5.15.0-122-generic
    - Intel(R) Xeon(R) CPU E5-2696 v3 @ 2.30GHz
    - Memory DDR3-ECC 2133MT/s 64G
    - GPU Tesla K80 24G GDDR5
编译环境：
    - g++ 11.4.0 -> c++23
    - g++ 10.5 -> c++17 (cuda compile)
    - nvcc 11.4
    - python3.7
中间件:
    - Redis 5.0.7
        - maxmemory 16gb
        - maxmemory-policy allkeys-lru
        - appendonly no
```

## 注：
### 1. nvcc 11.4版本编译不过，需临时注释掉以下：


- /usr/lib/gcc/x86_64-linux-gnu/11/include/serializeintrin.h:41
```c++
extern __inline void
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_serialize (void)
{
 // __builtin_ia32_serialize ();
}
```
- /usr/include/c++/11/bits/std_function.h:435
```c++
template<typename _Functor,
typename _Constraints = _Requires<_Callable<_Functor>>>
function(_Functor&& __f)
//noexcept(_Handler<_Functor>::template _S_nothrow_init<_Functor>())
: _Function_base()
```
- /usr/include/c++/11/bits/std_function.h:530
```c++
template<typename _Functor>
_Requires<_Callable<_Functor>, function&>
operator=(_Functor&& __f)
//noexcept(_Handler<_Functor>::template _S_nothrow_init<_Functor>())
{
function(std::forward<_Functor>(__f)).swap(*this);
return *this;
}
```
### 2. 开启线程亲和与优先级提升需root权限运行
