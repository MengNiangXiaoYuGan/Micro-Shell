/*
 * @brief 
 *
 * Custom MNXYG 
*/

#if !defined(FIFO_H)
#define FIFO_H

#ifdef __cplusplus
extern"C"{
#endif

/*ERROR CODE*/
#define FIFO_ERROR      1
#define FIFO_OVER_FLOW  4


/*fifo struct*/
enum FIFO_push_pull_type_t
{
    FIFO_PUSH_FORCE,     /*覆盖之前的数据强行存储*/
    FIFO_PUSH_REJECT     /*不覆盖数据，返回错误*/
};

struct FIFO_buff_t
{
    void *buff;             /*fifo buff*/
    unsigned short size;    /*fifo buff size*/
    unsigned short head; /*如果后面一个是tell，就满了，head不存储数据，它永远是空的,从0开始*/
    unsigned short tail; /*如果和head重叠，就是空的，tail在非空状态下一直都有数据，从0开始*/
};

/*
 * @brief 该宏用于静态声明一个指定大小的fifo，
 *      由于是静态声明，该宏仅应用于c文件及函数
 *      外，可在不跨函数的状态下在函数内使用，
 *      不可以在头文件内使用
 * @param x fifo名称
 * @param buff_size buff大小
 * @return NULL
*/
#define FIFO_CREATE_STATIC(x, buff_size) \
unsigned char buff_##x [buff_size + 1];\
struct FIFO_buff_t x = {\
    .buff = buff_##x,\
    .size = buff_size + 1,\
    .head = 0,\
    .tail = 0,\
}

/*
 * @brief 该宏用于初始化一个指定大小的fifo，
 *      需要传入一个外部的数组指针用于当作
 *      缓冲区，并且传入这个数组的大小
 * @param x fifo名称
 * @param buff 外部数组的地址，void类型
 * @param buff_size 外部数组的大小
 * @return NULL
 * @note 由于算法特性，实际可用的缓冲区大小是传入数组大小-1
*/
#define FIFO_CREATE(x, buff, buff_size) \
struct FIFO_buff_t x = {\
    .buff = buff,\
    .size = buff_size,\
    .head = 0,\
    .tail = 0\
};


/*
 * @brief 该宏用于初始化一个动态注册的指定大小的FIFO
 * @param x_ptr FIFO 的指针
 * @param buff_size 可用的缓存区大小
 * @return 0 is success
 * @return - is cannot malloc space
*/
#define FIFO_CREATE_MALLOC(x_ptr, buff_size) FIFO_Register_Malloc(x_ptr, buff_size)

/*
 * @brief 该宏用于释放FIFO内存，调用后，之前注册的fifo便不可使用
 * @param x_ptr FIFO的指针
 * @return NULL
*/
#define FIFO_DISTORY_MALLOC(x_ptr) FIFO_UnRegister_Malloc(x_ptr)

/*
 * @brief 该函数用于动态注册一个指定大小的fifo
 * @param fifo 指针
 * @param buff_size fifo大小
 * @return 0 is success
 * @return -FIFO_ERROR malloc失败
*/
int FIFO_Register_Malloc (struct FIFO_buff_t *fifo, unsigned short buff_size);

/*
 * @brief 该函数用于废弃动态注册的fifo
 * @param fifo 指针
 * @return NULL
*/
void FIFO_UnRegister_Malloc (struct FIFO_buff_t *fifo);

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
        unsigned short size);

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
        unsigned short size);

#ifdef __cplusplus
}
#endif
#endif // FIFO_H
