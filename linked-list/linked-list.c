/*
 * @brief linked list 
 *
 * Custom MNXYG 
 * 
*/

#include "linked-list.h"
#include "string.h"
#include "stdlib.h"

#ifndef NULL
#define NULL ((void*)0)
#endif //NULL

/*
 * @brief 该函数初始化一个单向链表节点，链表空间在内存申请
 * @param data_ptr 数据的指针
 * @return 链表的指针或者NULL
*/
Linked_List_Node_Single_D_t* Linked_List_Node_Init(void *data_ptr)
{
    Linked_List_Node_Single_D_t *ret = 
        (Linked_List_Node_Single_D_t *)malloc(sizeof(Linked_List_Node_Single_D_t));

    if (NULL == ret)
    {
        return NULL;
    }
    ret->next_node = NULL;
    ret->data = data_ptr;
    return ret;
}

/*
 * @brief 该函数遍历从该节点开始的所有节点，并且返回节点数和各节点的指针
 * @param node 节点指针
 * @param node_list 节点指针数组
 * @return 节点总数
 * @return - is error code
*/
int Linked_List_Foreach_Node (Linked_List_Node_Single_D_t *node, 
        Linked_List_Node_Single_D_t *node_list[])
{
    int ret = 0;
    Linked_List_Node_Single_D_t *ptr = node;
    /*To prevent a linked list loop, 
        use an extra pointer to indicate 
        that if during traversal the next 
        node is the same as the current node,
        return the size.*/
    const Linked_List_Node_Single_D_t *start = ptr;
    node_list[ret] = ptr;
    while (NULL != ptr->next_node)
    {
        ret ++;
        ptr = ptr->next_node;
        node_list[ret] = ptr;
        if (ptr == start)
        {
            break;
        }
    }
    ret ++;
    return ret;
}

/*
 * @brief 该函数将新的节点拼接在当前节点前面，并且返回新的节点作为头部
 * @param node 需要拼接的旧的节点的指针
 * @param new_node 新的节点
 * @return 新的节点的指针作为头
*/
Linked_List_Node_Single_D_t* Linked_List_Add_Head(Linked_List_Node_Single_D_t *node,
            Linked_List_Node_Single_D_t *new_node)
{
    new_node->next_node = (void *)node;
    return new_node;
}

/*
 * @brief 该函数将新的节点拼接在链表后面
 * @param node 需要拼接的旧的链表头的指针
 * @param node_new 需要拼接在尾部的链表节点的指针
 * @return 链表的头指针
*/
Linked_List_Node_Single_D_t* Linked_List_Add_Tail(Linked_List_Node_Single_D_t *node, 
        Linked_List_Node_Single_D_t *new_node)
{
    Linked_List_Node_Single_D_t *ptr = node;
    int ret = 1;
    while(NULL != ptr->next_node)
    {
        ret++;
        ptr = ptr->next_node;
    }
    ret++;
    ptr->next_node = new_node;
    return node;
}

/*
 * @brief 该函数从当前节点开始，遍历并删除整张链表
 * @param node 链表头
 * @return 删除链表节点的数量
*/
int Linked_List_Distory_All_List(Linked_List_Node_Single_D_t *node)
{
    int ret = 0;
    Linked_List_Node_Single_D_t *ptr = node;
    while (NULL != ptr->next_node)
    {
        ret++;
        Linked_List_Node_Single_D_t *del_ptr = ptr;
        ptr = ptr->next_node;
        free(del_ptr);
    }
    ret++;
    free(ptr);
    return ret;
}
