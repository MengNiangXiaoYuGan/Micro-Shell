/*
 * @brief 
 *
 * Custom MNXYG 
*/

#include "cache.h"
#include "stdlib.h"
#include "string.h"

#define    CACHE_NO_DATA    5
#define    CACHE_NO_MEM     6

/*******************************************API FUNCTION****************************************************** */

/*
 * @brief 该函数用于初始化一个Cache管理器，管理器会请求指定大小的空间用于缓存内容
 * @param cache
 * @param size
 * @return 0 is success
 * @return - is error code
*/
int Cache_Init (Cache_Handle_t *cache, unsigned int size)
{
    /*init memory broke*/
    cache->cache_base_addr = malloc(size);
    if (NULL == cache->cache_base_addr)
    {
        return -CACHE_NO_MEM;
    }
    memset(cache->cache_base_addr, 0, size);

    /*init internal value*/
    cache->HEAD = Linked_List_Node_Init(cache->cache_base_addr, 0);
    cache->cache_size = size;

    return 0;
}

/*
 * @brief 该函数用于向缓存内写入数据
 * @param cache
 * @param data
 * @param size
 * @return 0 is success
 * @return - is error code
*/
int Cache_push_data_to_buff(Cache_Handle_t *cache, void *data, unsigned int size)
{
    
}

/*
 * @brief 该函数用于归档缓存内数据
 * @param cache
 * @return 0 is success
*/
int Cache_Archive_buff(Cache_Handle_t *cache)
{

}

/*
 * @brief 该函数取出当前缓冲区内容
 * @param cache
 * @param data
 * @return >=0 is count of data size
 * @return - is error code
*/
inline int Cache_get_buff(Cache_Handle_t *cache, void *data)
{
    return Cache_get_history_by_index(cache, data, 0);
}

/*
 * @brief 该函数按照深度获取历史记录
 * @param cache
 * @param data
 * @param deep 历史记录深度，0为buff内容，相当于 Cache_get_buff 函数，数字增加，则历史记录越旧
 * @return >= 0 is count of data size
 * @return - is error code
*/
int Cache_get_history_by_index(Cache_Handle_t *cache, void *data, unsigned int deep)
{
    Linked_List_Node_t *ptr = cache->HEAD;
    for (unsigned int i = 0; i < deep; i++)
    {
        if (NULL != ptr->next_node)
        {
            ptr = ptr->next_node;
        }
        else
        {
            return -CACHE_NO_DATA;
        }
    }

    /*这里我不喜欢，这种实现方式必须要求外面有足够大的缓存，但我这个才是缓存啊*/
    if (ptr->data + ptr->size > cache->cache_base_addr + cache->cache_size)
    {
        /*cut to 2 phase*/
        const unsigned int relese_size = (cache->cache_base_addr + cache->cache_size) - (ptr->data + ptr->size);
        const unsigned int cut_size = (ptr->size - relese_size);
        memcpy(data, ptr->data, cut_size);
        memcpy(data + cut_size, cache->cache_base_addr, relese_size);
    }
    else
    {
        memcpy(data, ptr->data, ptr->size);
    }

    return cache->HEAD->size;
}

/*
 * @brief 该函数用于弃用cache，清理内存
 * @param cache
 * @return NULL
*/
void Cache_DeInit(Cache_Handle_t *cache)
{
    /*distory history*/
    Linked_List_Distory_All_List(cache->HEAD);

    /*distory cache*/
    free(cache->cache_base_addr);
    cache->cache_size = 0;
}
