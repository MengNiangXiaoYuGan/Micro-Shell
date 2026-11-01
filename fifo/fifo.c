/*
 * @brief 
 *
 * Custom MNXYG 
*/

#include "fifo.h"
#include "stdlib.h"

/*
 * @brief 该函数用于动态注册一个指定大小的fifo
 * @param fifo 指针
 * @param buff_size fifo大小
 * @return 0 is success
 * @return -FIFO_ERROR malloc失败
*/
int FIFO_Register_Malloc (struct FIFO_buff_t *fifo, unsigned short buff_size)
{
    fifo->buff = malloc(buff_size + 1);
    if (NULL == fifo->buff)
    {
        return -FIFO_ERROR;
    }
    fifo->size = buff_size + 1;
    fifo->head = 0;
    fifo->tail = 0;
    return 0;
}

/*
 * @brief 该函数用于废弃动态注册的fifo
 * @param fifo 指针
 * @return NULL
*/
void FIFO_UnRegister_Malloc (struct FIFO_buff_t *fifo)
{
    free(fifo->buff);
}

/*
 * @brief 该函数用于往FIFO里存储数据
 * @param fifo 存储区结构体的指针
 * @param mode 压入的方式
 * @param data 数据指针
 * @param size 数据长度
 * @return 0 存储成功
 * @param - is error code
*/
int FIFO_push_data (struct FIFO_buff_t *fifo,
        enum FIFO_push_pull_type_t mode,
        void *data,
        unsigned short size)
{
    /*check size*/
    unsigned short release_size = 0;
    if (fifo->head >= fifo->tail)
    {
        /*还没有到头，size是尾大小加上头前面的部分 - 1*/
        release_size = fifo->size - fifo->head + fifo->tail - 1;
    }
    else
    {
        /*已经转了一圈了，size是尾减去头 - 1*/
        release_size = fifo->tail - fifo->head - 1;
    }
    if (size > release_size)
    {
        switch (mode)
        {
        case FIFO_PUSH_FORCE:
            /*let the tail push*/
            for (unsigned short i = 0; i < size - release_size; i++)
            {
                fifo->tail++;
                if (fifo->tail >= fifo->size)
                {
                    fifo->tail = 0;
                }
            }
            break;
        case FIFO_PUSH_REJECT:
            return -FIFO_OVER_FLOW;
        default:
            return -FIFO_ERROR;
            break;
        }
    }
    unsigned char *buff = (unsigned char *)fifo->buff;
    unsigned char *data_ptr = (unsigned char *)data;
    for (unsigned short i = 0; i < size; i++)
    {
        buff[fifo->head] = data_ptr[i];
        fifo->head++;
        if (fifo->head >= fifo->size)
        {
            fifo->head = 0;
        }
    }
    return 0;
}

/*
 * @brief 该函数用于从队列中取出数据
 * @param fifo 缓冲区句柄
 * @param data 输出数据的指针
 * @param size 要取出多少数据
 * @return 0 没有取出数据，缓冲区是空的
 * @return >0 实际取出了多少数据
 * @return - is error code
*/
int FIFO_pull_data(struct FIFO_buff_t *fifo,
        void *data,
        unsigned short size)
{
    /*check fifo no empety*/
    if (fifo->tail == fifo->head)
    {
        return 0;
    }
    /*check how much data can get*/
    unsigned short release_size = 0;
    if (fifo->head > fifo->tail)
    {
        /*not around*/
        release_size = fifo->head - fifo->tail;
    }
    else
    {
        release_size = (fifo->size - fifo->tail) + fifo->head;
    }
    unsigned char ret = release_size >= size ? size : release_size;
    unsigned char *buff = (unsigned char *)fifo->buff;
    unsigned char *data_ptr = (unsigned char *)data;
    for (unsigned short i = 0; i < ret; i++)
    {
        data_ptr[i] = buff[fifo->tail];
        fifo->tail++;
        if (fifo->tail >= fifo->size)
        {
            fifo->tail = 0;
        }
    }
    return ret;
}