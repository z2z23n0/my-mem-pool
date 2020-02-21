#include "my_ngx_mem_pool.h"

// 创建指定size大小的内存池，但是小块内存池不超过一个页面的大小
void* ngx_mem_pool::ngx_create_pool(size_t size)
{
    ngx_pool_s* p;

    p = (ngx_pool_s*)malloc(size);
    if(p == nullptr)
    {
        return nullptr;
    }

    p->d.last = (u_char*)p + sizeof(ngx_pool_s);
    p->d.end = (u_char*)p + size;
    p->d.next = nullptr;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_s);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = nullptr;
    p->cleanup = nullptr;

    pool = p;

    return p;
}

// 考虑内存字节对齐，从内存池申请size大小的内存
void* ngx_mem_pool::ngx_palloc(size_t size)
{
    if(size <= pool->max)
    {
        return ngx_palloc_small(size, 1);
    }
    return ngx_palloc_large(size);
}

// 不考虑内存字节对齐，从内存池申请size大小的内存
void* ngx_mem_pool::ngx_pnalloc(size_t size)
{
    if(size <= pool->max)
    {
        return ngx_palloc_small(size, 0);
    }
    return ngx_palloc_large(size);
}

// 调用的是ngx_palloc实现内存分配，但是会初始化0
void* ngx_mem_pool::ngx_pcalloc(size_t size)
{
    void* p;
    p = ngx_palloc(size);
    if(p)
    {
        ngx_memzero(p, size);
    }
    return p;
}

// 小块内存分配
void* ngx_mem_pool::ngx_palloc_small(size_t size, ngx_uint_t align)
{
    u_char*     m;
    ngx_pool_s* p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }

        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;
            return m;
        }

        p = p->d.next;
    }while (p);

    return ngx_palloc_block(size);
}

// 分配新的小块内存池
void* ngx_mem_pool::ngx_palloc_block(size_t size)
{
    u_char*     m;
    size_t      psize;
    ngx_pool_s* p;
    ngx_pool_s* newpool;

    psize = (size_t)(pool->d.end - (u_char*)pool);

    m = (u_char*)malloc(psize);
    if(m == nullptr)
    {
        return nullptr;
    }

    newpool = (ngx_pool_s*)m;

    newpool->d.end = m + psize;
    newpool->d.next = nullptr;
    newpool->d.failed = 0;

    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    newpool->d.last = m + size;

    for(p = pool->current; p->d.next; p = p->d.next)
    {
        if(p->d.failed++ > 4)
        {
            pool->current = p->d.next;
        }
    }

    p->d.next = newpool;

    return m;
}

// 大块内存分配
void* ngx_mem_pool::ngx_palloc_large(size_t size)
{
    void*               p;
    ngx_uint_t          n;
    ngx_pool_large_s*   large;

    p = malloc(size);
    if(p == nullptr)
    {
        return nullptr;
    }

    n = 0;

    for(large = pool->large; large; large = large->next)
    {
        if(large->alloc == nullptr)
        {
            large->alloc = p;
            return p;
        }

        if(n++ > 3)
        {
            break;
        }
    }

    large = (ngx_pool_large_s*)ngx_palloc_small(sizeof(ngx_pool_large_s), 1);
    if(large == nullptr)
    {
        free(p);
        return nullptr;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

// 释放大块内存
void ngx_mem_pool::ngx_pfree(void *p)
{
    ngx_pool_large_s* l;
    for(l = pool->large; l; l = l->next)
    {
        if(p == l->alloc)
        {
            free(l->alloc);
            l->alloc = nullptr;
            return;
        }
    }
}

// 内存重置函数
void ngx_mem_pool::ngx_reset_pool()
{
    ngx_pool_s*         p;
    ngx_pool_large_s*   l;

    for(l = pool->large; l; l = l->next)
    {
        if(l->alloc)
        {
            free(l->alloc);
        }
    }

    // 处理第一个块内存池
    p = pool;
    p->d.last = (u_char*)p + sizeof(ngx_pool_s);
    p->d.failed = 0;

    // 从第二块内存池开始循环到最后一个内存池
    for(p = p->d.next; p; p = p->d.next)
    {
        p->d.last = (u_char*)p + sizeof(ngx_pool_data_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->large = nullptr;
}

// 内存池的销毁函数
void ngx_mem_pool::ngx_destory_pool()
{
    ngx_pool_s*             p;
    ngx_pool_s*             n;
    ngx_pool_large_s*       l;
    ngx_pool_cleanup_s*     c;

    for(c = pool->cleanup; c; c = c->next)
    {
        if(c->handler)
        {
            c->handler(c->data);
        }
    }

    for(l = pool->large; l ; l = l->next)
    {
        if(l->alloc)
        {
            free(l->alloc);
        }
    }

    for(p = pool, n = pool->d.next; ; p = n, n = n->d.next)
    {
        free(p);
        if(n == nullptr)
        {
            break;
        }
    }
}

// 添加回调清理操作函数
ngx_pool_cleanup_s* ngx_mem_pool::ngx_pool_cleanup_add(size_t size)
{
    ngx_pool_cleanup_s*     c;

    c = (ngx_pool_cleanup_s*)ngx_palloc(sizeof(ngx_pool_cleanup_s));
    if(c == nullptr)
    {
        return nullptr;
    }

    if(size)
    {
        c->data = ngx_palloc(size);
        if(c->data == nullptr)
        {
            return nullptr;
        }
    }
    else
    {
        c->data = nullptr;
    }

    c->handler = nullptr;
    c->next = pool->cleanup;
    pool->cleanup = c;

    return c;
}
