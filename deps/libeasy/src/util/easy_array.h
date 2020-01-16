#ifndef EASY_ARRAY_H_
#define EASY_ARRAY_H_

/**
 * 固定长度数组分配
 * 内部维护了一个内存池，因为元素为固定长度，故在删除元素后，为了
 * 重用已分配的内存，将其加入内部的list中，并优先进行下次分配
 * 为了能够加入list，限制objsize最小也得是easy_list_t的大小
 */
#include "easy_pool.h"
#include "easy_list.h"

EASY_CPP_START

typedef struct easy_array_t {
    easy_pool_t             *pool;
    easy_list_t             list;
    int                     object_size;
    int                     count;
} easy_array_t;

easy_array_t *easy_array_create(int object_size);
void easy_array_destroy(easy_array_t *array);
void *easy_array_alloc(easy_array_t *array);
void easy_array_free(easy_array_t *array, void *ptr);

EASY_CPP_END

#endif
