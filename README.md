# my-mem-pool

![language](https://img.shields.io/badge/language-c++-DeepPink.svg) [![GitHub license](https://img.shields.io/github/license/YuzeZhang/my-mem-pool.svg)](https://github.com/YuzeZhang/my-mem-pool/blob/master/LICENSE) [![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

通过剖析开源代码可以积累优秀的代码设计思想和良好的编程规范，了解不同应用场景下不同的内存池实现也是一种重要的能力，本仓库对SGI STL二级空间配置器源码和nginx内存池源码进行了剖析和注释，并将其分别移植成简单易用的形式。

本仓库包含以下内容：

1. 注释过的SGI STL二级空间配置器源码以及对其的分析整理
2. 注释过的nginx内存池源码以及对其的分析整理
3. my_allocator
4. my_nginx

## Table of Contents

- [背景](#Background)
- [使用](#Usage)
- [使用许可](#License)

## Background

在学习编程的过程中，一味的闭门造车是不可取的，就和作家为什么要看书一样，积累优秀资源是创造优秀资源的必要条件。

“所谓创意，只是把原有的元素重新组合而已。”

通过对SGI STL二级空间配置器源码的剖析和对nginx内存池源码的剖析来学习内存池的设计，并且体会SGI STL二级空间配置器和nginx中的内存池在设计上的区别，不同的应用场景选择不同的内存池的设计实现。

## Usage

## License

[MIT © YuzeZhang.](https://github.com/YuzeZhang/my-mem-pool/blob/master/LICENSE)