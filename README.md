# my-mem-pool

![language](https://img.shields.io/badge/language-c++-DeepPink.svg) [![GitHub license](https://img.shields.io/github/license/YuzeZhang/my-mem-pool.svg)](https://github.com/YuzeZhang/my-mem-pool/blob/master/LICENSE) [![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

通过剖析开源代码可以积累优秀的代码设计思想和良好的编程规范，了解不同应用场景下不同的内存池实现也是一种重要的能力，本仓库对SGI STL二级空间配置器源码和nginx内存池源码进行了剖析，并使用C++ OOP进行仿写，使其便于移植到其它项目当中。

本仓库包含以下内容：

1. 注释过的SGI STL二级空间配置器源码以及对其的分析整理
2. 注释过的nginx内存池源码以及对其的分析整理
3. my_stl_allocator
4. my_nginx_mem_pool

## Table of Contents

- [背景](#Background)
- [使用](#Usage)
- [TODO](#TODO)
- [使用许可](#License)

## Background

在学习编程的过程中，一味的闭门造车是不可取的，就和作家为什么要看书一样，积累优秀资源是创造优秀资源的必要条件。

“所谓创意，只是把原有的元素重新组合而已。”

通过对SGI STL二级空间配置器源码的剖析和对nginx内存池源码的剖析来学习内存池的设计，并且体会SGI STL二级空间配置器和nginx中的内存池在设计上的区别，不同的应用场景选择不同的内存池的设计实现。

## Usage

- [查看注释的SGI STL配置器源码](https://github.com/YuzeZhang/my-mem-pool/tree/master/annotated_STL_allocator)

- [查看SGI STL配置器源码剖析](https://github.com/YuzeZhang/my-mem-pool/tree/master/annotated_STL_allocator)

- [查看注释的nginx内存池源码](https://github.com/YuzeZhang/my-mem-pool/tree/master/annotated_nginx_allocator)

- [查看nginx内存池源码剖析](https://github.com/YuzeZhang/my-mem-pool/tree/master/annotated_nginx_allocator)

- [使用my_ngx_mem_pool](https://github.com/YuzeZhang/my-mem-pool/tree/master/my_nginx_mem_pool)

  ![my_ngx_mem_pool](https://zhangyuzechn.cn/wp-content/uploads/2020/02/nginx.gif)

- [使用my_stl_allocator](https://github.com/YuzeZhang/my-mem-pool/tree/master/my_stl_allocator)

  ![my_stl_allocator](https://zhangyuzechn.cn/wp-content/uploads/2020/02/allocator.gif)

## TODO

- [ ] ngx_create_pool的代码逻辑可以直接实现在ngx_mem_pool的构造函数中
- [ ] ngx_destory_pool的代码逻辑可以直接实现在ngx_mem_pool的析构函数中

## License

[MIT © YuzeZhang.](https://github.com/YuzeZhang/my-mem-pool/blob/master/LICENSE)