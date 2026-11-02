/*
 * @brief 
 *
 * Custom MNXYG 
 * 
 * License: 
*/

#if !defined(CACHE_H)
#define CACHE_H

#include "../linked-list/linked-list.h"

/*
 * @brief 该组件是Micor Shell的依赖，用于管理字符缓存和历史记录，该组件依赖linked list
 *          
 *          我有个更好的想法，当归档缓存的时候，再申请内存，这样只需要限制历史记录条目最大值即可
 *  
 *          其具体的实现方法是：申请一个整体内存块，通过一个双向非循环链表实现字符缓存，
 *          在链表中，链表头代表当前字符缓冲，在没有历史记录的情况下，该头的字符缓冲大小
 *          可以占据整个内存块，当用户调用压入函数的时候，就会将指定大小的数据压入cache中
 *          ，同样地，用户也能从当前cache中取出数据，取出函数将会返回历史记录的头指针和
 *          历史记录大小，有一个额外的链表指针用于指示需要调用的历史记录。
 *          随着用户调用归档函数，当前cache头内容会被归档，一个新的链表头会插在cache头前面，
 *          当用户不断归档历史记录，可用的cache空间就会越来越小，直至cache碰到最旧的历史记录，
 *          此时最旧的历史记录就会被删除，并且清空那部分的内存给cache使用，直到cache碰到自己的
 *          头形成回环，此时cache爆栈，返回错误后清除整个cache。下方是一个简单的图示，表明了该组件是如何工作的
 * 
 * 
 *          base addr                                                       cache size + base addr
 *          |       |       |       |       |       |       |       |       |       |       |
 *          --------------------------------------------------------------------------------    
 *          |
 *          head
 * 
 *          storage some thing
 *          |hello world    
 *          |           |
 *          head        size
 *          
 *          archive
 *          |hello world|
 *          his1<------>head
 * 
 *          storage more and mor somthing
 *          |hello world|aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa|
 *          his<------->head                                                                size
 * 
 *          and next
 *          |aaaaaa     |aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa|
 *                size  head
 * 
 *          and
 *          |aaaaaaaaaaa|aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa|
 *                   sizehead
 *      
 *          now cache is full, system will crlean all cache
 *          |           |                                                                   |
 *                      head
 * 
 *  注意，该库需要消耗大量的堆栈空间用于存放历史记录索引和历史记录本身，注意设备堆栈大小
 * 
 */

typedef struct
{

    void *cache_base_addr;

    unsigned int cache_size;

    Linked_List_Node_t *HEAD;

}Cache_Handle_t;

/*
 * @brief 该函数用于初始化一个Cache管理器，管理器会请求指定大小的空间用于缓存内容
 * @param cache
 * @param size
 * @return 0 is success
 * @return - is error code
*/
int Cache_Init (Cache_Handle_t *cache, unsigned int size);

/*
 * @brief 该函数用于向缓存内写入数据
 * @param cache
 * @param data
 * @param size
 * @return 0 is success
 * @return - is error code
*/
int Cache_push_data_to_buff(Cache_Handle_t *cache, void *data, unsigned int size);

/*
 * @brief 该函数用于归档缓存内数据
 * @param cache
 * @return 0 is success
*/
int Cache_Archive_buff(Cache_Handle_t *cache);

/*
 * @brief 该函数取出当前缓冲区内容
 * @param cache
 * @param data
 * @return >=0 is count of data size
 * @return - is error code
*/
int Cache_get_buff(Cache_Handle_t *cache, void *data);

/*
 * @brief 该函数按照深度获取历史记录
 * @param cache
 * @param data
 * @param deep 历史记录深度，0为buff内容，相当于 Cache_get_buff 函数，数字增加，则历史记录越旧
 * @return >= 0 is count of data size
 * @return - is error code
*/
int Cache_get_history_by_index(Cache_Handle_t *cache, void *data, unsigned int deep);

/*
 * @brief 该函数用于弃用cache，清理内存
 * @param cache
 * @return NULL
*/
void Cache_DeInit(Cache_Handle_t *cache);


#endif // CACHE_H
