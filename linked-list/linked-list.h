/*
 * @brief linked list
 *
 * Custom MNXYG 
 * 
*/

/*
 * 该库实现了双向，非循环链表的创建，头插，尾插，整体删除的功能
 * 双向链表可以直接当作单向链表使用，只要不去在意prev_node即可
*/

#if !defined(LINKED_LIST)
#define LINKED_LIST

/*use for single direction linked list*/
typedef struct Linked_List_Node_t
{
    struct Linked_List_Node_t *next_node;       /*next node addr*/
    void *data;                                 /*this node data*/
    int size;                                   /*this is node data size (option)*/
    struct Linked_List_Node_t *prev_node;       /*next node addr*/
}Linked_List_Node_t;

/*
 * @brief 该函数初始化一个单向链表节点，链表空间在内存申请
 * @param data_ptr 数据的指针
 * @return 链表的指针或者NULL
*/
Linked_List_Node_t* Linked_List_Node_Init(void *data_ptr, int size);

/*
 * @brief 该函数遍历从该节点开始的所有节点，并且返回节点数和各节点的指针
 * @param node 节点指针
 * @param node_list 节点指针数组
 * @return 节点总数
 * @return - is error code
*/
int Linked_List_Foreach_Node (Linked_List_Node_t *node, 
        Linked_List_Node_t *node_list[]);

/*
 * @brief 该函数将新的节点拼接在当前节点前面，并且返回新的节点作为头部
 * @param node 需要拼接的旧的节点的指针
 * @param new_node 新的节点
 * @return 新的节点的指针作为头
*/
Linked_List_Node_t* Linked_List_Add_Head(Linked_List_Node_t *node,
        Linked_List_Node_t *new_node);

/*
 * @brief 该函数将新的节点拼接在链表后面
 * @param node 需要拼接的旧的链表头的指针
 * @param node_new 需要拼接在尾部的链表节点的指针
 * @return 链表的头指针
*/
Linked_List_Node_t* Linked_List_Add_Tail(Linked_List_Node_t *node, 
        Linked_List_Node_t *new_node);

/*
 * @brief 该函数从当前节点开始，遍历并删除整张链表
 * @param node 链表头
 * @return 删除链表节点的数量
*/
int Linked_List_Distory_All_List(Linked_List_Node_t *node);

#endif // LINKED_LIST
