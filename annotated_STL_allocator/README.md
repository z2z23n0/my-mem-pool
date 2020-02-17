> 通过剖析SGI STL二级空间配置器内存池源码深入理解其实现原理

注：由于2016年惠普（HPE）收购SGI后SGI官网便不再提供STL源码下载，所以这里也不提供STL全部的源码，只有配置器模块所涉及的部分，来源于[steveLauwh/SGI-STL](https://github.com/steveLauwh/SGI-STL)

## 概念

### 配置器

负责空间配置与管理，从实现的角度来看，配置器是一个实现了动态空间配置、空间管理、空间释放的class template。

### 空间配置器

整个STL的操作对象（所有的数值）都存放在容器之内，而容器一定需要配置空间以存放内容。

### 空间配置器核心作用

分离了对象的内存开辟，对象构造

分离了对象的内存释放，对象析构

### 空间配置器的分级

SGI STL包含了一级空间配置器和二级空间配置器，其中一级空间配置器allocator采用malloc和free来管理内存，和C++标准库中提供的allocator是一样的，但其二级空间配置器allocator采用了基于freelist自由链表原理的内存池机制实现内存管理。

## 空间配置器源码分析

### SGI STL空间配置器的结构

SGI STL的配置器，其名称是alloc而不是allocator，而且不接受任何参数。

SGI STL 的每一个容器都已经指定其缺省的空间配置器为 alloc。

```c++
template <class T, class Alloc = alloc>  // 缺省使用 alloc 为配置器
class vector {...};

vector<int, std::alloc> iv; 
```

- <defalloc.h>----SGI 标准的空间配置器，std::allocator

  allocator 只是基层内存配置/释放行为(::operator::new 和 ::operator::delete)的一层薄薄的包装，并没有考虑到任何效率上的强化。

- SGI 特殊的空间配置器，std::alloc

  - <stl_construct.h>：定义了全局函数 construct() 和 destroy()，负责对象的构造和析构。
  - <stl_alloc.h>：定义了一、二级配置器，配置器名为 alloc。
  - <stl_uninitialized.h>：定义了全局函数，用来填充(fill)或复制(copy)大块内存数据。

- 构造和析构基本工具

  具体看 <stl_construct.h> 源码，功能是构造和析构操作。

- 空间的配置和释放，std::alloc

  - 向 system heap 要求空间
  - 考虑多线程(multi-threads)状态
  - 考虑内存不足时的应变措施
  - 考虑过多 “小型区块” 可能造成的内存碎片问题

  对象构造前的空间配置 和 对象析构后的空间释放，具体看 <stl_alloc.h>

### SGI STL空间配置器的分析

考虑到小型区块可能造成内存碎片问题，SGI 采用两级配置器，第一级配置器直接使用 malloc() 和 free() 实现；第二级配置器使用 memory pool 内存池管理。

第二级配置器的原理：

- 当配置区块超过 128 bytes，就使用第一级配置器
- 当配置区块小于 128 bytes，使用内存池管理

```c++
enum {_ALIGN = 8};  // 小型区块的上调边界
enum {_MAX_BYTES = 128}; // 小区区块的上限
enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN  free-list 的个数

// free-list 的节点结构，降低维护链表 list 带来的额外负担
union _Obj {
    union _Obj* _M_free_list_link;  // 利用联合体特点
    char _M_client_data[1];    /* The client sees this. */
};
static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS];  // 注意，它是数组，每个数组元素包含若干相等的小额区块
```

其中 free-list 是指针数组，16 个数组元素，就是 16 个 free-list，各自管理大小分别为 8， 16， 24， 32，...128 bytes(8 的倍数)的小额区块。

小额区块的结构体 `union _Obj` 使用链表连接起来。

配置器负责配置，同时也负责回收。

[![img](https://github.com/steveLauwh/SGI-STL/raw/master/The%20Annotated%20STL%20Sources%20V3.3/Other/allocator_memorypool.PNG)](https://github.com/steveLauwh/SGI-STL/raw/master/The Annotated STL Sources V3.3/Other/allocator_memorypool.PNG)

### 空间配置器的相关定义

```c++
template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class vector : protected _Vector_base<_Tp, _Alloc>
```

可以看到，容器的默认空间配置器是__STL_DEFAULT_ALLOCATOR( _Tp)，它是一个宏定义，如下：

```c++
# ifndef __STL_DEFAULT_ALLOCATOR
# ifdef __STL_USE_STD_ALLOCATORS
# define __STL_DEFAULT_ALLOCATOR(T) allocator< T >
# else
# define __STL_DEFAULT_ALLOCATOR(T) alloc
# endif
# endif
```

从上面可以看到__STL_DEFAULT_ALLOCATOR通过宏控制有两种实现，一种是allocator< T >，另一种是alloc，这两种分别就是SGI STL的一级空间配置器和二级空间配置器的实现。

```c++
template <int __inst>
class __malloc_alloc_template {// 一级空间配置器内存管理类 -- 通过malloc和free管理内存
```

```c++
template <bool threads, int inst>
class __default_alloc_template { // 二级空间配置器内存管理类 -- 通过自定义内存池实现内存管理
```

### 重要类型和变量定义

```c++
// 内存池的粒度信息
enum {_ALIGN = 8};
enum {_MAX_BYTES = 128};
enum {_NFREELISTS = 16};
```

```c++
// 每一个内存chunk块的头信息
union _Obj {
union _Obj* _M_free_list_link;
char _M_client_data[1]; /* The client sees this. */
};
```

```c++
// 组织所有自由链表的数组，数组的每一个元素的类型是_Obj*，全部初始化为0
static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS];
// Chunk allocation state. 记录内存chunk块的分配情况
static char* _S_start_free;
static char* _S_end_free;
static size_t _S_heap_size;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;
```

### 重要的辅助接口函数

```c++
/*将 __bytes 上调至最邻近的 8 的倍数*/
static size_t _S_round_up(size_t __bytes)
{ return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }
```

```c++
/*返回 __bytes 大小的chunk块位于 free-list 中的编号*/
static size_t _S_freelist_index(size_t __bytes) {
return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1); }
```

### 内存池管理函数

```c++
// 分配内存的入口函数
static void* allocate(size_t __n)
    
// 负责把分配好的chunk块进行连接，添加到自由链表当中
static void* _S_refill(size_t __n);

// 分配相应内存字节大小的chunk块，并且给下面三个成员变量初始化
static char* _S_chunk_alloc(size_t __size, int& __nobjs);

// 把chunk块归还到内存池
static void deallocate(void* __p, size_t __n);

// 内存池扩容函数
template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void* __p,
                                                    size_t __old_sz,
                                                    size_t __new_sz);
```

