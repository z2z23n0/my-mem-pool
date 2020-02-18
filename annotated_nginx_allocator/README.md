> 剖析nginx的内存池源码，理解原理实现以及该内存池设计的应用场景

注：源码来源于[y123456yz/reading-code-of-nginx-1.9.2](https://github.com/y123456yz/reading-code-of-nginx-1.9.2)

## 概述

nginx使用内存池对内存进行管理，内存管理的实现与SGI STL空间配置器有类似之处，都把内存分配归结为大内存分配和小内存分配。若申请的内存大小比同页的内存池最大值 max 还大，则是大内存分配，否则为小内存分配。

其主要流程如下图，其中size是用户请求分配内存的大小，pool是现有内存池。

![nginx内存分配流程图](https://zhangyuzechn.cn/wp-content/uploads/2020/02/nginx内存池流程图.png)

## 内存池源码分析

### 重要类型定义

```c++
// nginx内存池的主结构体类型
struct ngx_pool_s {
ngx_pool_data_t d; // 内存池的数据头
size_t max; // 小块内存分配的最大值
ngx_pool_t *current; // 小块内存池入口指针
ngx_chain_t *chain;
ngx_pool_large_t *large; // 大块内存分配入口指针
ngx_pool_cleanup_t *cleanup; // 清理函数handler的入口指针
ngx_log_t *log;
};
```

```c++
typedef struct ngx_pool_s ngx_pool_t;
// 小块内存数据头信息
typedef struct {
u_char *last; // 可分配内存开始位置
u_char *end; // 可分配内存末尾位置
ngx_pool_t *next; // 保存下一个内存池的地址
ngx_uint_t failed; // 记录当前内存池分配失败的次数
} ngx_pool_data_t;
```

```c++
typedef struct ngx_pool_large_s ngx_pool_large_t;
// 大块内存类型定义
struct ngx_pool_large_s {
ngx_pool_large_t *next; // 下一个大块内存
void *alloc; // 记录分配的大块内存的起始地址
};
```

```c++
typedef void (*ngx_pool_cleanup_pt)(void *data); // 清理回调函数的类型定义

typedef struct ngx_pool_cleanup_s ngx_pool_cleanup_t;
// 清理操作的类型定义，包括一个清理回调函数，传给回调函数的数据和下一个清理操作的地址
struct ngx_pool_cleanup_s {
ngx_pool_cleanup_pt handler; // 清理回调函数
void *data; // 传递给回调函数的指针
ngx_pool_cleanup_t *next; // 指向下一个清理操作
};
```

### nginx内存池重要函数接口

```c++
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log); // 创建内存池
void ngx_destroy_pool(ngx_pool_t *pool); // 销毁内存池
void ngx_reset_pool(ngx_pool_t *pool); // 重置内存池
void *ngx_palloc(ngx_pool_t *pool, size_t size); // 内存分配函数，支持内存对齐
void *ngx_pnalloc(ngx_pool_t *pool, size_t size); // 内存分配函数，不支持内存对齐
void *ngx_pcalloc(ngx_pool_t *pool, size_t size); // 内存分配函数，支持内存初始化0
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p // 内存释放（大块内存）
```

```c++
ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size); // 添加清理handler
```

## 参考资料

[https://blog.csdn.net/chenhanzhun/article/details/42365605](https://blog.csdn.net/chenhanzhun/article/details/42365605)

[https://blog.csdn.net/v_JULY_v/article/details/7040425](https://blog.csdn.net/v_JULY_v/article/details/7040425)