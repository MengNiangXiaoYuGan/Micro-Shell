/*
 * @brief 
 *
 * Custom MNXYG 
 * 
*/

#include "micro-shell.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define ANSI_FONT_BOLD              "\x1B[1m"       /*加粗*/
#define ANSI_FONT_ITALIC            "\x1B[3m"       /*斜体*/
#define ANSI_FONT_UNDERLINE         "\x1B[4m"       /*下划线*/
#define ANSI_FONT_HIGHLIGHT         "\x1B[7m"       /*反显*/
#define ANSI_FONT_STRIKETHROUGH     "\x1B[9m"       /*删除线*/

#define ANSI_COLOR_RED   "\x1B[31m"
#define ANSI_COLOR_GRN   "\x1B[32m"
#define ANSI_COLOR_YEL   "\x1B[33m"
#define ANSI_COLOR_BLU   "\x1B[34m"
#define ANSI_COLOR_MAG   "\x1B[35m"
#define ANSI_COLOR_CYN   "\x1B[36m"
#define ANSI_COLOR_WHT   "\x1B[37m"
#define ANSI_COLOR_RESET "\x1B[0m"

#define ANSI_NONE        ""
#define ANSI_END_CODE    '\0'

#define SHELL_HEAD(x,ret,string)        Terminal_Print((x),"%s%d "ANSI_COLOR_RESET ANSI_FONT_BOLD ANSI_COLOR_MAG"%s" ANSI_COLOR_RESET "->%s",   \
                                    (ret) == 0 ? ANSI_COLOR_GRN : ANSI_COLOR_RED,                                                               \
                                    (ret),                                                                                                      \
                                    (x)->host_name,                                                                                             \
                                    (string))

#define SHELL_BACK_TO_HEAD(x)       Terminal_Print((x), "\r\033[K")
#define SHELL_BACK_SPACE(x)         Terminal_Print((x), "\b \b")

#define SHELL_HELLO_INTERFACE(x)    \
        Terminal_Print((x), "*******************************************************\r\n"\
                            "****************____MICRO_____SHELL____****************\r\n"\
                            "*******************************************************\r\n")

#define SHELL_NEW_LINE(x)           Terminal_Print((x),"\r\n")
#define SHELL_CLEAN_SCREEN(x)       Terminal_Print((x),"\033[H\033[2J")


#define SHELL_INF(x,...)            Terminal_Print((x),ANSI_COLOR_GRN "<inf> : %s\r\n" ANSI_COLOR_RESET, ##__VA_ARGS__)
#define SHELL_ERR(x,...)            Terminal_Print((x),ANSI_COLOR_RED "<err> : %s\r\n" ANSI_COLOR_RESET, ##__VA_ARGS__)

#if !defined(UNUSED)
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif // UNUSED   

/*****************************************DEFAULT SHELL FUNCTION**************************************** */

/*
 * @brief 该函数用以测试参数导出功能是否正常
*/
int cmd_test (int argc,void *argv[])
{
    /*get shell handle*/
    Shell_Handle_t *shell = (Shell_Handle_t *)argv[0];
    /*get this command steurct*/
    const Shell_Command_Typedef *cmd = Shell_Serch_Command(shell, argv[1]);
    /*get param list and num*/
    int param_num = Shell_Complite_Param_To_Num(cmd, argv[2]);

    if (param_num < 0)  /*get not fetch any param*/
    {
        unsigned int i;
        Terminal_Print (shell, "argc=%d ",argc);
        Terminal_Print (shell, "argv = [");
        for (i = 1; i < argc; i++)
        {
            Terminal_Print(shell, "%s, ",(char *)argv[i]);
        }
        Terminal_Print(shell, "%s]\r\n",(char *)argv[i]);
    }
    else if (param_num == 0)    /*is the fix param of "--help" or "-h"*/
    {
        /*can put any wont to display function of help on there*/
        Terminal_Print(shell, "%-16s%-16s%-24s\r\n", 
            cmd->params[param_num].full_name, 
            cmd->params[param_num].nick_name, 
            cmd->params[param_num].help);
    }
    return 0;
}
SHELL_COMMAND_DEFINE(test, "Show all params", cmd_test)

/*
 * @brief 该函数用于显示所有命令和命令对应的帮助
*/
int cmd_help (int argc,void *argv[])
{
    UNUSED(argc);
    Shell_Handle_t *shell = (Shell_Handle_t *)argv[0];
    Shell_Command_Typedef *command_list[shell->command_num];
    if (Shell_Foreach_Command(shell, command_list))
    {
        SHELL_ERR(shell, "cannot foreach all command");
        return -SHELL_ERROR;
    }
    for (unsigned short i = 0; i < shell->command_num; i++)
    {
        Terminal_Print(shell, "%-10s %s\r\n",command_list[i]->name,command_list[i]->params[0].help);
    }
    return 0;
}
SHELL_COMMAND_DEFINE(help,"Show command list", cmd_help)

/*
 * @brief 该函数用于清屏
*/
int cmd_clean (int argc,void *argv[])
{
    UNUSED(argc);
    Shell_Handle_t *shell = (Shell_Handle_t *)argv[0];
    SHELL_CLEAN_SCREEN(shell);
    return 0;
}
SHELL_COMMAND_DEFINE(clean, "clean screen", cmd_clean)
SHELL_COMMAND_DEFINE(clear, "clean screen", cmd_clean)

/*****************************************HELPER FUNCTION************************************************ */

/*
 * @brief 该函数用于向终端发送格式化字符串在，实际使用中
 *          这个函数将完全替代printf的功能
*/
void Terminal_Print(Shell_Handle_t *shell,
    const char* format,...)
{
    char temp[4096];
    va_list canshu;
    int data_num = 0;
    va_start(canshu,format);
    data_num = vsprintf(temp,format,canshu);
    va_end(canshu);
    shell->terminal_send_message(temp, data_num);
}

/*
 * @brief 该函数用于获取一个字符串内用分隔符分割的各个参数，并且将参数组回传
 * @param message   收到的字符串
 * @param delim     分隔符
 * @param get       参数组
 * @param max_num   最多参数数量
 * @return 分割后的参数数量
*/
static short Terminal_Get_Param(char *message,
        char *delim,
        void *get[],
        short max_number)
{
    short i;
    void *ptr = NULL;
    ptr = strtok(message,delim);
    for (i = 0; ptr != NULL && i < max_number; i++)
    {
        get[i] = ptr;
        ptr = strtok(NULL,delim);
    }
    return(i);
}

/*
 * @brief 该函数用于回传命令列表的大小，
 *         也就是命令的数量
 * @param shell
 * @return number of command
*/
inline unsigned char Shell_get_Command_List_Size(Shell_Handle_t *shell)
{
    return shell->command_num;
}

/*
 * @brief 该函数用于归档历史记录
 * @param shell
 * @return 0 is success
 * @return nagetive history linked list have loop
*/
int Shell_Archive_History (Shell_Handle_t *shell)
{
    /*check cache limit*/
    while (shell->cache_size > shell->history_size_limit - shell->history_size)
    {
        /*kill most older history*/
        Linked_List_Node_t *node = shell->history;
        Linked_List_Node_t *start = node;
        while (NULL != node->next_node->next_node)
        {
            node = node->next_node;
            if (node->next_node == start)
            {
                return -SHELL_ERROR;
            }
        }
        free(node->next_node->data);
        shell->cache_size -= node->next_node->size;
        free(node->next_node);
        node->next_node = NULL;
    }
    /*archive this buff*/
    void *buff_ptr = malloc(shell->cache_size);
    if (NULL == buff_ptr)
    {
       return -SHELL_NO_MEM;
    }
    memcpy(buff_ptr, shell->cache_buff, shell->cache_size);
    Linked_List_Node_t *his = Linked_List_Node_Init(buff_ptr, shell->cache_size);
    shell->history = Linked_List_Add_Head(shell->history, his);

    /*reset buff*/
    memset(shell->cache_buff, ANSI_END_CODE, sizeof(shell->cache_buff));
    shell->cache_size = 0;
    return 0;
}

/*
 * @brief 该函数按照深度获取历史记录
 * @param shell
 * @param data
 * @param deep 历史记录深度，0为第一条buff
 * @return >= 0 is count of data size
 * @return -n is error code
 */
int Shell_get_history_by_index(Shell_Handle_t *shell, void **data, unsigned int deep)
{
    Linked_List_Node_t *ptr = shell->history;
    if (NULL == ptr)
    {
        return -SHELL_BUSY;
    }
    
    for (unsigned int i = 0; i < deep; i++)
    {
        if (NULL != ptr->next_node)
        {
            ptr = ptr->next_node;
        }
        else
        {
            return -SHELL_NO_FETCH;
        }
    }
    *data = ptr->data;
    return ptr->size;
}

/*
 * @brief 该函数用于遍历命令链表，并且返回命令列表
 * @param shell
 * @param command_list
 * @return 0 is success
 * @return - is error
*/
int Shell_Foreach_Command(Shell_Handle_t *shell, Shell_Command_Typedef *command_list[])
{
    Linked_List_Node_t *list[shell->command_num];
    int ret = Linked_List_Foreach_Node(shell->Shell_command, list);
    if (ret != shell->command_num)
    {
        return -SHELL_ERROR;
    }
    for (unsigned short i = 0; i < shell->command_num; i++)
    {
        command_list[i] = (Shell_Command_Typedef *)list[i]->data;
    }
    return 0;
}

/*
 * @brief 该函数用于遍历命令链表，搜索对应名称的命令，并将命令结构返回
 * @param shell
 * @param name
 * @return 命令结构的指针或者NULL
*/
const Shell_Command_Typedef* Shell_Serch_Command(Shell_Handle_t *shell, char* name)
{
    Linked_List_Node_t *list[shell->command_num];
    int ret = Linked_List_Foreach_Node(shell->Shell_command, list);
    if (ret != shell->command_num)
    {
        return NULL;
    }
    for (unsigned short i = 0; i < shell->command_num; i++)
    {
        const Shell_Command_Typedef *cmd = (const Shell_Command_Typedef *)list[i]->data;
        if (strcmp(name,cmd->name) == 0)
        {
            return cmd;
        }
    }
    return NULL;
}

/*
 * @brief 该函数用于获取参数的顺序值，用于匹配次级参数
 * @param cmd 该命令结构体的指针
 * @param param 该命令参数的字符串
 * @return 参数的顺序值，从0开始，0为定义的第一个参数
 * @return - is error
 * @note 该函数为用户可以直接在自定义命令里使用的辅助函数，通过判断返回的值，即可知道传入的次级参数是什么
*/
int Shell_Complite_Param_To_Num(const Shell_Command_Typedef *cmd, char *param)
{
    const Shell_Param_Typedef *param_list = cmd->params;
    int ret  = 0;
    unsigned char shoot = 0;
    for (ret = 0; ret < cmd->param_num; ret++)
    {
        if (strcmp(param_list[ret].full_name, param) == 0 || strcmp(param_list[ret].nick_name, param) == 0)
        {
            shoot = 1;
            break;
        }
    }
    if (shoot == 0)
    {
        return -SHELL_NO_FETCH;
    }
    return ret;
}

//自动补全(只完成了关联搜索，不会自动修改当前补全档)
/*
 * @brief 该函数用于进行函数补全，并且在下方显示出来，该函数不会修改当前缓冲区内的数据
 * @param shell 被调用的shell的句柄
 * @param cache_data    缓冲区内数据
 * @param cache_size    缓冲区长度
 * @return 0 is success
 * @return - is error code
*/
static int Terminal_Function_Complite (Shell_Handle_t *shell, 
        char *cache_data,
        unsigned int cache_size)
{
    SHELL_NEW_LINE(shell);
    char secend_cache[cache_size + 1];
    for (unsigned int i = 0; i < cache_size; i++)
    {
        secend_cache[i] = cache_data[i];
    }
    secend_cache[cache_size] = '\0';
    //dilime params
    void *params[SHELL_MAX_PARAM_NUMBER];
    unsigned char param_number = Terminal_Get_Param(secend_cache," ",params,SHELL_MAX_PARAM_NUMBER);
    Shell_Command_Typedef *command_list[shell->command_num];
    if (Shell_Foreach_Command(shell, command_list))
    {
        SHELL_ERR(shell, "cannot foreach all command");
        return -SHELL_ERROR;
    }
    if(cache_size)
    {
        //search name
        char shot = 0;
        if (param_number > 1)
        {

        }
        else 
        {
            for (unsigned short i = 0; i < shell->command_num; i++)
            {
                if (strncmp((char *)params[0],command_list[i]->name,strlen((char *)params[0])) == 0)
                {
                    Terminal_Print(shell,"%-8s ",command_list[i]->name);
                    shot = 1;
                }
            }
            SHELL_NEW_LINE(shell);
        }
        
        if(!shot) return -SHELL_NO_FETCH;
    }
    else
    {
        /*not any input but push tab, just print all command to screen let user select*/
        for (unsigned short i = 0; i < shell->command_num; i++)
        {
            Terminal_Print(shell, "%-8s ",command_list[i]->name);
        }
        SHELL_NEW_LINE(shell);
        /*display all command*/
    }
    return 0;
}

//esc相关-多字节解析-方向键或者翻页键
/*
 * @brief ESC相关，多字节解析，
 * @param shell shell句柄
 * @param data 单个字节
 * @return 0 is success
 * @return -SHELL_BUSY 没解析完成或者解析结果没有用处
 * @return -SHELL_ERROR 解析错误
*/
static int Terminal_ESC_Mulit_Byte_Anlayze (Shell_Handle_t *shell,
        char data)
{
    char *data_buf = NULL;
    if (data == 0x1b)                    //esc is pushed
    {
        shell->mulit_key_status = 1;
        return -SHELL_BUSY;
    }
    else if (data == 0x5b && shell->mulit_key_status == 1)
    {
        shell->mulit_key_status = 2;
        return -SHELL_BUSY;
    }
    else if (shell->mulit_key_status == 2)
    {
        unsigned int tmp_entry = shell->history_entry;
        switch (data)
        {
        case 'A':          //up bottom
            if (tmp_entry <= 65536)
            {
                tmp_entry++;
            }
            /*display history*/
            if (Shell_get_history_by_index(shell, (void *)&data_buf, tmp_entry - 1) < 0)
            {
                return -SHELL_NO_FETCH;
            }
            SHELL_BACK_TO_HEAD(shell);
            SHELL_HEAD(shell, shell->ret, data_buf);
            break;
        case 'B':          //dowm bottom
            if (tmp_entry <= 1)
            {
                /*display buff*/
                SHELL_BACK_TO_HEAD(shell);
                SHELL_HEAD(shell, shell->ret, shell->cache_buff);
                break;
            }
            else
            {
                tmp_entry--;
            }
            /*display history*/
            if (Shell_get_history_by_index(shell, (void *)&data_buf, tmp_entry - 1) < 0)
            {
                return -SHELL_NO_FETCH;
            }
            SHELL_BACK_TO_HEAD(shell);
            SHELL_HEAD(shell, shell->ret, data_buf);
            break;
        case 'C':          //right bottom
            // Terminal_Print(hfoc,"right\r\n");
            break;
        case 'D':          //left bottom
            // Terminal_Print(hfoc,"left\r\n");
            break;
        default:
            shell->mulit_key_status = 0;
            break;
        }
        shell->history_entry = tmp_entry;
        shell->mulit_key_status = 0;
        return 1;
    }
    else
    {
        shell->mulit_key_status = 0;
    }
    return 0;
}

/**************************************USER FUNCTION*************************************** */

/*
 * @brief 该函数初始化Shell层，Shell可以在没有foc实例的状态下独立运行
 * @param shell 终端的虚拟句柄
 * @param terminal_send_message 发送API函数，需要用户实现，用于发送字符串
 * @return 0
 * @return -SHELL_ERROR   初始化失败
*/
int Shell_Iint (Shell_Handle_t *shell,
            void (*send_data)(char *message,unsigned short size))
{
    if (NULL == send_data)
    {
        return -SHELL_ERROR;
    }
    shell->terminal_send_message = send_data;

    /*init data cache*/
    strncpy(shell->host_name, SHELL_HOST_NAME, sizeof(SHELL_HOST_NAME));
    memset(shell->cache_buff, ANSI_END_CODE, sizeof(shell->cache_buff));
    shell->cache_size = 0;

    /*init history*/
    shell->history_entry = 0;
    shell->history = NULL;
    shell->history_size_limit = SHELL_CACHE_HISTORY;
    shell->history_size = 0;

    /*init fifo*/
    if (FIFO_CREATE_MALLOC(&shell->fifo, SHELL_CACHE_DEEP/2))
    {
        SHELL_ERR(shell, "no memory to give fifo");
        return -SHELL_NO_MEM;
    }

    /*init screen*/
    shell->ret = 0;
    SHELL_CLEAN_SCREEN(shell);
    SHELL_HELLO_INTERFACE(shell);
    SHELL_HEAD(shell,shell->ret,ANSI_NONE);
    shell->mulit_key_status = 0;

    /*register command*/
    shell->command_num = 0;
    shell->Shell_command = NULL;
    if (Shell_Register_Command(shell, &help))
    {
        SHELL_ERR(shell, "no memory to register function %d", shell->command_num);
        return -SHELL_NO_MEM;
    }
    if (Shell_Register_Command(shell, &clean))
    {
        SHELL_ERR(shell, "no memory to register function %d", shell->command_num);
        return -SHELL_NO_MEM;
    }
    if (Shell_Register_Command(shell, &clear))
    {
        SHELL_ERR(shell, "no memory to register function %d", shell->command_num);
        return -SHELL_NO_MEM;
    }
    if (Shell_Register_Command(shell, &test))
    {
        SHELL_ERR(shell, "no memory to register function %d", shell->command_num);
        return -SHELL_NO_MEM;
    }
    return 0;
}

/*
 * @brief 该函数将会进行一次函数注册，将传入的用户函数
 *        注册到命令表内
 * @param shell
 * @param command
 * @return 0 is success
 * @return - is error
 * @note 注意，传入的是命令的指针
 *       该函数必须在Shell_Init调用后才能调用，否则会出现命令表回环无法遍历的情况
*/
int Shell_Register_Command(Shell_Handle_t *shell,const Shell_Command_Typedef *command)
{
    Linked_List_Node_t *ptr = Linked_List_Node_Init((void *)command, command->param_num * sizeof(Shell_Param_Typedef));
    if (NULL == ptr)
    {
        return -SHELL_NO_MEM;
    }
    shell->Shell_command = Linked_List_Add_Head(shell->Shell_command, ptr);
    shell->command_num++;
    return 0;
}

/*
 * @brief 该函数将数据推入内部环形缓冲区，处理函数会在非中断条件下处理这些数据
 * @param shell 终端的虚拟句柄
 * @param data 数据头指针
 * @param size 数据长度
 * @return 0
 * @return -SHELL_OVER_RANGE  超出缓冲区长度
*/
int Shell_push_data(Shell_Handle_t *shell,
            char *data, 
            unsigned char size)
{
    if (FIFO_push_data(&shell->fifo, FIFO_PUSH_REJECT, data, size) < 0)
    {
        return -SHELL_OVER_RANGE;
    }
    return 0;
}

/*
 * @brief 该函数用于循环运行shell解算和命令处理，每调用一次理论上最多能处理32字节的数据
 *       该函数处理后会自动退出，如果需要在操作系统环境下将该函数当作线程运行的话，需要额外的
 *      死循环
 * @param shell 终端的虚拟句柄
 * @return 0
 * @return -SHELL_BUSY 缓冲区是空的，没有东西需要处理
 * @return -SHELL_ERROR 输入的命令触发的错误
*/
int Shell_Task (Shell_Handle_t *shell)
{
    char data[2];
    unsigned char size = FIFO_pull_data(&shell->fifo, data, 1);
    if (size == 0)
    {
        return 0;
    }

    /*check mulite input like up down key*/
    if (Terminal_ESC_Mulit_Byte_Anlayze(shell, data[0]))
    {
        return 0;
    }

    switch (data[0])                //fine speshel key
    {
    case 0x08:
    case 0x7f:                      //back space or delate
        /*delete form cache*/
        if (shell->cache_size > 0)
        {
            shell->cache_size--;
            shell->cache_buff[shell->cache_size] = ANSI_END_CODE;
            SHELL_BACK_SPACE(shell);
        }
        return -SHELL_BUSY;
        break;
    case 0x09:                      //tab is be pushed haha
        Terminal_Function_Complite(shell,
                shell->cache_buff,
                shell->cache_size);
        SHELL_HEAD(shell, shell->ret, shell->cache_buff);
        return -SHELL_BUSY;
        break;
    case 0x10:
    case 0x0d:                      //enter or shift enter
        shell->cache_buff[shell->cache_size] = ANSI_END_CODE;
        shell->cache_size++;
        SHELL_NEW_LINE(shell);
        /*only this option can continue nest stage*/
        break;
    case 0x1b:                      //esc is pushed
        return -SHELL_NO_FETCH;
        break;
    default:                        //notthing juet nomal data
        /*save to cache buff*/
        if (shell->cache_size < sizeof(shell->cache_buff))
        {
            shell->cache_buff[shell->cache_size] = data[0];
            shell->cache_size++;
            Terminal_Print(shell, "%s", data);
            return 0;
        }
        else return -SHELL_OUT_OF_CACH;
        break;
    }

    /*now checke command*/
    /*get buff*/
    if(shell->cache_size > 1)
    {
        //dilime params
        void *params[SHELL_MAX_PARAM_NUMBER + 1];
        params[0] = (void *)shell;
        short params_number = Terminal_Get_Param(shell->cache_buff, " ", &params[1], SHELL_MAX_PARAM_NUMBER);
        //search functuin
        const Shell_Command_Typedef *cmd = Shell_Serch_Command(shell,(char *)params[1]);
        if (NULL == cmd)
        {
            Terminal_Print(shell, "%s : can`t not fine command\r\n",params[1]);
            shell->ret = -SHELL_NO_FETCH;
        }
        else
        {
            shell->ret = cmd->terminal_function(params_number,params);
        }
        //archive cache to histiory
        if (Shell_Archive_History(shell) < 0)
        {
            SHELL_ERR(shell, "cache history is loop or memory out");
        }
    }
    else
    {
        shell->ret = 0;
        shell->cache_size = 0;
    }
    //anthe loop steart
    shell->history_entry = 0;
    SHELL_HEAD(shell, shell->ret, ANSI_NONE);
    return 0;
}
