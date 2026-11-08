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

#define ANSI_NONE                   ""
#define ANSI_END_CODE               '\0'

#define ANSI_RIGHT                  "\033[C"
#define ANSI_LEFT                   "\033[D"
#define ANSI_BACK_INS(x, ptr)       Terminal_Print((x), "%s \033[%dD",(ptr), strlen(ptr) + 1)

#define SHELL_HEAD(x,ret,string)        Terminal_Print((x),"%s%d "ANSI_COLOR_RESET ANSI_FONT_BOLD ANSI_COLOR_MAG"%s" ANSI_COLOR_RESET "->%s",   \
                                    (ret) == 0 ? ANSI_COLOR_GRN : ANSI_COLOR_RED,                                                               \
                                    (ret),                                                                                                      \
                                    (x)->host_name,                                                                                             \
                                    (string))

#define SHELL_BACK_TO_HEAD(x)       Terminal_Print((x), "\r\033[K")
#define SHELL_BACK_SPACE(x)         Terminal_Print((x), "\b \b")
#define SHELL_INSTE_STR(x, string)  Terminal_Print((x),"\033[@%s", string)

#define SHELL_HELLO_INTERFACE(x)    \
        Terminal_Print((x), "*******************************************************\r\n"\
                            "****************____MICRO_____SHELL____****************\r\n"\
                            "*******************************************************\r\n")

#define SHELL_CLEAN_BUFF(x)         memset((x)->cache_buff, 0, sizeof((x)->cache_buff));(x)->cache_size=0
#define SHELL_CLEAN_CURSOR(x)       (x)->cursor_position=0

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

    for (int i = 2; i <= argc; i++)
    {
        int param_num = Shell_Complite_Param_To_Num(cmd, argv[2]);
        switch (param_num)
        {
        case 0:
            /*display help information*/
            Shell_Display_Command_Help(shell, cmd);
            return 0;
            break;
        case 1:
            if (i + 1 <= argc)
            {
                i++;
                int emp = 0;
                if (Shell_Conversion_Str_to_Int(argv[i], &emp) < 0)
                {
                    SHELL_ERR(shell, "cannot vovertion this params");
                    return -SHELL_NO_FETCH;
                }
                Terminal_Print(shell, "conversion value is %d\r\n", emp);
            }
            else
            {
                SHELL_ERR(shell, "not more argument");
                return -SHELL_NO_FETCH;
            }
            break;
        case 2:
            if (i + 1 <= argc)
            {
                i++;
                float emp = 0;
                if (Shell_Conversion_Str_to_Float(argv[i], &emp) < 0)
                {
                    SHELL_ERR(shell, "cannot vovertion this params");
                    return -SHELL_NO_FETCH;
                }
                Terminal_Print(shell, "conversion value is %f\r\n", emp);
            }
            else
            {
                SHELL_ERR(shell, "not more argument");
                return -SHELL_NO_FETCH;
            }
            break;
        case 3:
            if (i + 1 <= argc)
            {
                i++;
                int emp = 0;
                emp = Shell_Conversion_Str_to_Bool(argv[i], "true", "false");
                if (emp < 0)
                {
                    SHELL_ERR(shell, "cannot vovertion this params");
                    return -SHELL_NO_FETCH;
                }
                Terminal_Print(shell, "conversion value is %d, type %s\r\n", emp, emp == 0 ? "false" : "true");
            }
            else
            {
                SHELL_ERR(shell, "not more argument");
                return -SHELL_NO_FETCH;
            }
            break;
        default:
            Terminal_Print (shell, "argc=%d ",argc);
            Terminal_Print (shell, "argv = [");
            int a;
            for (a = 1; a < argc; a++)
            {
                Terminal_Print(shell, "%s, ",(char *)argv[a]);
            }
            Terminal_Print(shell, "%s]\r\n",(char *)argv[a]);
            return 0;
            break;
        }
    }
    return 0;
}
SHELL_COMMAND_DEFINE(test, "Show all params", cmd_test,
    SHELL_PARAM_DEFINE("--change_int", "-i", "change input to integer")
    SHELL_PARAM_DEFINE("--change_flo", "-f", "change input to float")
    SHELL_PARAM_DEFINE("--change_bool", "-b", "change input to bool, use true or false")
)

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
    char temp[SHELL_POP_CACHE_DEEP];
    va_list canshu;
    int data_num = 0;
    va_start(canshu,format);
    data_num = vsnprintf(temp, SHELL_POP_CACHE_DEEP, format, canshu);
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
static short Shell_Get_Param(char *message,
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
 * @brief 该函数用于清除最旧的历史记录（尾删）
 *         不直接使用linked-list库的尾删函数
 *         的主要原因是历史记录的data同样使用
 *         malloc获取，需要free，而linked-list
 *         的尾删函数没有这个功能
 * @param shell
 * @return 0 is success
 * @return -SHELL_NOMEM 历史记录已经删光了
 * @return -SHELL_ERROR 历史记录回环了（在实际程序中这不应该出现）
*/
static int Shell_Delete_Oldest_History(Shell_Handle_t *shell)
{
    if (shell->history == NULL)
    {
        /*if no more history list*/
        return -SHELL_NO_MEM;
    }
    else if (shell->history->next_node == NULL)
    {
        /*last one history*/
        shell->history_size -= (shell->history->size + sizeof(Linked_List_Node_t));
        free(shell->history->data);
        free(shell->history);
        shell->history = NULL;
    }
    else
    {
        /*kill older history*/
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
        shell->history_size -= (node->next_node->size + sizeof(Linked_List_Node_t));
        free(node->next_node->data);
        free(node->next_node);
        node->next_node = NULL;
    }
    return 0;
}

/*
 * @brief 该函数用于归档历史记录
 * @param shell
 * @return 0 is success
 * @return nagetive history linked list have loop
*/
int Shell_Archive_History (Shell_Handle_t *shell)
{
    int ret = 0;
    /*check cache limit*/
    while (shell->cache_size > shell->history_size_limit - shell->history_size)
    {
        /*kill most older history*/
        ret = Shell_Delete_Oldest_History(shell);
        if (ret < 0)
        {
            return ret;
        }
    }
    /*archive this buff*/
    void *buff_ptr = malloc(shell->cache_size);
    while (NULL == buff_ptr)
    {
        /*kill most older history*/
        ret = Shell_Delete_Oldest_History(shell);
        if (ret < 0)
        {
            return ret;
        }
        buff_ptr = malloc(shell->cache_size);
    }
    memcpy(buff_ptr, shell->cache_buff, shell->cache_size);
    Linked_List_Node_t *his = Linked_List_Node_Init(buff_ptr, shell->cache_size);
    if (NULL == his)
    {
        free(buff_ptr);
        return -SHELL_NO_MEM;
    }
    shell->history = Linked_List_Add_Head(shell->history, his);
    shell->history_size += (shell->cache_size + sizeof(Linked_List_Node_t));
    return 0;
}

/*
 * @brief 该函数按照深度获取历史记录
 * @param shell
 * @param data
 * @param deep 历史记录深度，1为最新的历史记录，输入0无效
 * @return >= 0 is count of data size
 * @return -n is error code
 */
int Shell_get_history_by_index(Shell_Handle_t *shell, void **data, unsigned int deep)
{
    if (deep == 0)
    {
        return -SHELL_NO_FETCH;
    }

    Linked_List_Node_t *ptr = shell->history;
    if (NULL == ptr)
    {
        return -SHELL_BUSY;
    }
    
    for (unsigned int i = 0; i < deep - 1; i++)
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

/*
 * @brief 该函数用于显示命令的帮助，由于定义命令的时候会默认定义一个--help以及-h的参数
 *        ，当用户不想为编写help处理函数段的时候，这个函数可以提供一个标准并且格式化的
 *        显示方案，同时，这个函数在输入完整命令的情况下，敲击tab键，就会触发，显示当前
 *        命令的帮助文档。
 * @param shell
 * @param cmd
 * @return 0
 * @note 该命令的执行结果会在shell上打印如下的内容,以名称为cmd，并且有两个自定义参数的
 *       自定义命令为例：
 *          cmd         this is cmd help
 *                  --help      -h      use for display informations
 *                  --load      -l      load settings
 *                  --save      -s      save settings
*/
int Shell_Display_Command_Help (Shell_Handle_t *shell, const Shell_Command_Typedef *cmd)
{
    Terminal_Print(shell, "%-12s %s\r\n", cmd->name, cmd->params[0].help);
    Terminal_Print(shell, "params : \r\n");
    Terminal_Print(shell, "%-8d%-12s %-8s %s\r\n", 0, cmd->params[0].full_name, cmd->params[0].nick_name, "use for display help informations");
    for (unsigned char i = 1; i < cmd->param_num; i++)
    {
        Terminal_Print(shell, "%-8d%-12s %-8s %s\r\n", i, cmd->params[i].full_name, cmd->params[i].nick_name, cmd->params[i].help);
    }
    return 0;
}

/*
 * @brief 该函数用于将输入的参数转为整型
 * @param param
 * @param num
 * @return 0 is success
 * @return nagetive is error code
*/
int Shell_Conversion_Str_to_Int (char *param, int *num)
{
    /*this fuction only can opration 10 base int*/
    /*need more opration 0x and 1e3 like number*/
    char *ptr  = param;
    int nagetive = 1;
    *num = 0;
    /*jump space*/
    while (*ptr == ' ')ptr++;
    /*check the first str*/
    if (*ptr == '-')
    {
        nagetive = -1;
        ptr++;
    }
    else if(*ptr == '+')
    {
        ptr++;
    }
    /*the first number*/
    while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9' )
    {
        *num = (*num * 10) + (*ptr - '0');
        ptr++;
    }
    if (ptr == param)
    {
        *num = 0;
        return -SHELL_ERROR;
    }
    else if (*ptr != '\0')
    {
        *num = 0;
        return -SHELL_NO_FETCH;
    }
    *num *= nagetive;
    return 0;
}

/*
 * @brief 该函数用于将输入参数转换为浮点型
 * @param param
 * @param num
 * @return 0 is success
 * @return nagetive is error code
*/
int Shell_Conversion_Str_to_Float (char *param, float *num)
{
    char *ptr  = param;
    float nagetive = 1;
    *num = 0;
    /*jump space*/
    while (*ptr == ' ')ptr++;
    /*check the first str*/
    if (*ptr == '-')
    {
        nagetive = -1;
        ptr++;
    }
    else if(*ptr == '+')
    {
        ptr++;
    }
    while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9' )
    {
        *num = (*num * 10.f) + (*ptr - '0');
        ptr++;
    }
    if (ptr == param)
    {
        *num = 0;
        return -SHELL_ERROR;
    }
    else if (*ptr == '.')
    {
        /*look is '.' ?*/
        ptr++;
        float xiaoshu = 0.1f;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9' )
        {
            *num += (*ptr - '0') * xiaoshu;
            xiaoshu *= 0.1;
            ptr++;
        }
    }
    if (*ptr != '\0')
    {
        return -SHELL_NO_FETCH;
    }
    *num *= nagetive;
    return 0;
}

/*
 * @brief 该函数用于将输入参数转为bool型
 * @param param
 * @param true_type 判断为true的字符串
 * @param false_type 判断为false的字符串
 * @return 0 false
 * @return 1 true
 * @return nagative is error code
*/
int Shell_Conversion_Str_to_Bool (char *param, const char *true_type, const char *false_type)
{
    if (strcmp((const char *)param, true_type) == 0)
    {
        return 1;
    }
    else if (strcmp((const char *)param, false_type) == 0)
    {
        return 0;
    }
    else
    {
        return -SHELL_NO_FETCH;
    }
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
    unsigned char param_number = Shell_Get_Param(secend_cache," ",params,SHELL_MAX_PARAM_NUMBER);
    Shell_Command_Typedef *command_list[shell->command_num];
    if (Shell_Foreach_Command(shell, command_list))
    {
        SHELL_ERR(shell, "cannot foreach all command");
        return -SHELL_ERROR;
    }
    if(cache_size)
    {
        const Shell_Command_Typedef *tmp_cmd = Shell_Serch_Command(shell, (char *)params[0]);
        //search name
        char shot = 0;
        if (param_number > 1)
        {
            /*complite params*/
            if (tmp_cmd == NULL) return -SHELL_NO_FETCH;
            for (unsigned int  i = 0; i < tmp_cmd->param_num; i++)
            {
                if (strncmp(tmp_cmd->params[i].full_name, (char *)params[param_number - 1], strlen((char *)params[param_number -1])) == 0 ||
                        strncmp(tmp_cmd->params[i].nick_name,(char *)params[param_number - 1], strlen((char *)params[param_number -1])) == 0)
                {
                    /*print params and helps*/
                    Terminal_Print(shell, "%-8d%-12s %-8s %s\r\n", i, tmp_cmd->params[i].full_name, tmp_cmd->params[i].nick_name, tmp_cmd->params[i].help);
                    shot++;
                }
            }
        }
        else
        {
            /*foreach command loke full complte*/
            if (tmp_cmd == NULL)
            {
                /*only one param number mens one full complet or half compliet*/
                for (unsigned short i = 0; i < shell->command_num; i++)
                {
                    if (strncmp((char *)params[0],command_list[i]->name,strlen((char *)params[0])) == 0)
                    {
                        Terminal_Print(shell,"%-8s ",command_list[i]->name);
                        shot++;
                    }
                }
            }
            else
            {
                Shell_Display_Command_Help(shell, tmp_cmd);
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
        int error_code = 0;
        switch (data)
        {
        case 'A':          //up bottom
            if (shell->history_entry <= 65536)
            {
                shell->history_entry++;
            }
            /*display history*/
            error_code = Shell_get_history_by_index(shell, (void *)&data_buf, shell->history_entry);
            if (error_code < 0)
            {
                shell->history_entry--;
                shell->mulit_key_status = 0;
                return -SHELL_NO_FETCH;
            }
            shell->cache_size = error_code - 1;
            shell->cursor_position = shell->cache_size;
            SHELL_BACK_TO_HEAD(shell);
            SHELL_HEAD(shell, shell->ret, data_buf);
            break;
        case 'B':          //dowm bottom
            if (shell->history_entry > 0)
            {
                shell->history_entry--;
            }
            if (shell->history_entry == 0)
            {
                /*display buff*/
                SHELL_BACK_TO_HEAD(shell);
                SHELL_HEAD(shell, shell->ret, shell->cache_buff);
                shell->cache_size = strlen(shell->cache_buff);
                shell->cursor_position = shell->cache_size;
                break;
            }
            else
            {
                /*display history*/
                error_code = Shell_get_history_by_index(shell, (void *)&data_buf, shell->history_entry);
                if (error_code < 0)
                {
                    shell->history_entry++;
                    shell->mulit_key_status = 0;
                    return -SHELL_NO_FETCH;
                }
                shell->cache_size = error_code- 1;
                shell->cursor_position = shell->cache_size;
                SHELL_BACK_TO_HEAD(shell);
                SHELL_HEAD(shell, shell->ret, data_buf);
                break;
            }
            break;
        case 'C':          //right
            if (shell->cursor_position < shell->cache_size)
            {
                shell->cursor_position++;
                Terminal_Print(shell, ANSI_RIGHT);
            }
            break;
        case 'D':          //left
            if (shell->cursor_position > 0)
            {
                shell->cursor_position--;
                Terminal_Print(shell, ANSI_LEFT);
            }
            break;
        default:
            shell->mulit_key_status = 0;
            break;
        }
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
int Shell_Init (Shell_Handle_t *shell,
            void (*send_data)(char *message,unsigned short size))
{
    if (NULL == send_data)
    {
        return -SHELL_ERROR;
    }
    shell->terminal_send_message = send_data;

    /*init data cache*/
    memset(shell->host_name, 0, sizeof(shell->host_name));
    strcpy(shell->host_name, SHELL_HOST_NAME);
    SHELL_CLEAN_BUFF(shell);
    SHELL_CLEAN_CURSOR(shell);

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
    Shell_Register_Command(shell, &help);
    Shell_Register_Command(shell, &clean);
    Shell_Register_Command(shell, &clear);
    Shell_Register_Command(shell, &test);
    return 0;
}

/*
 * @brief 该函数将会进行一次函数注册，将传入的用户函数
 *        注册到命令表内
 * @param shell
 * @param command
 * @return 0 is success
 * @return negetive is error
 * @note 注意，传入的是命令的指针
 *       该函数必须在Shell_Init调用后才能调用，否则会出现命令表回环无法遍历的情况
*/
int Shell_Register_Command(Shell_Handle_t *shell, Linked_List_Node_t *command)
{
    if (shell->command_num >= 255)
    {
        shell->ret = -SHELL_NO_MEM;
        SHELL_NEW_LINE(shell);
        SHELL_ERR(shell, "command numbers max, reject register");
        SHELL_HEAD(shell, shell->ret, ANSI_NONE);
        return -SHELL_NO_MEM;
    }
    Linked_List_Node_t *ptr = shell->Shell_command;
    if (NULL ==ptr)
    {
        shell->Shell_command = Linked_List_Add_Head(shell->Shell_command, command);
        shell->command_num++;
    }
    else
    {
        for (unsigned char i = 0; i < shell->command_num; i++)
        {
            if (ptr == command)
            {
                /*this command is register now*/
                shell->ret = -SHELL_COMMAND_EXITS;
                SHELL_NEW_LINE(shell);
                SHELL_ERR(shell, "this commander was registed");
                SHELL_HEAD(shell, shell->ret, ANSI_NONE);
                return -SHELL_COMMAND_EXITS;
            }
            ptr = ptr->next_node;
        }
        shell->Shell_command = Linked_List_Add_Head(shell->Shell_command, command);
        shell->command_num++;
    }
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
    /*this fuction is special witch is return -SHELL_BUSY*/
    /*that mean funtcion need next bytes to comferm */
    /*and return 1 mead function now opreation once*/
    if (Terminal_ESC_Mulit_Byte_Anlayze(shell, data[0]))
    {
        return -SHELL_BUSY;
    }

    /*if turn history, copy it to cache*/
    if (shell->history_entry)
    {
        /*get history buf and size*/
        char *tmp_of_history = NULL;
        int tmp_of_size = 0;
        SHELL_CLEAN_BUFF(shell);
        tmp_of_size = Shell_get_history_by_index(shell, (void *)&tmp_of_history, shell->history_entry);
        if (tmp_of_size < 0)
        {
            SHELL_NEW_LINE(shell);
            SHELL_ERR(shell, "tmp history can not get form heap, system error");
            SHELL_NEW_LINE(shell);
            SHELL_HEAD(shell, shell->ret, ANSI_NONE);
            return -SHELL_NO_MEM;
        }
        memcpy(shell->cache_buff, tmp_of_history, tmp_of_size - 1);
        /*cursor position is now settings*/
        shell->cache_size = (unsigned int)(tmp_of_size - 1);


        // shell->cursor_position = shell->cache_size;
        shell->history_entry = 0;
    }

    switch (data[0])                //fine speshel key
    {
    case 0x08:
    case 0x7f:                      //back space or delate
        /*delete form cache*/
        if (shell->cursor_position > 0 && shell->cursor_position == shell->cache_size)
        {
            shell->cache_size--;
            shell->cursor_position--;
            shell->cache_buff[shell->cache_size] = ANSI_END_CODE;
            SHELL_BACK_SPACE(shell);
        }
        else if (shell->cursor_position > 0)
        {
            shell->cursor_position--;
            /*put force data back*/
            SHELL_BACK_SPACE(shell);
            for (unsigned int i = shell->cursor_position; i < shell->cache_size; i++)
            {
                shell->cache_buff[i] = shell->cache_buff[i+1];
            }
            shell->cache_size--;
            char *ptr = &shell->cache_buff[shell->cursor_position];
            ANSI_BACK_INS(shell, ptr);
        }
        return -SHELL_BUSY;
        break;
    case 0x09:                      //tab is be pushed haha
        Terminal_Function_Complite(shell,
                shell->cache_buff,
                shell->cache_size);
        SHELL_HEAD(shell, shell->ret, shell->cache_buff);
        shell->cursor_position = shell->cache_size;
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
        if (shell->cache_size < sizeof(shell->cache_buff) - 1)
        {
            if (shell->cache_size != shell->cursor_position)
            {
                /*push data to tail*/
                for (unsigned int i = shell->cache_size; i > shell->cursor_position; i--)
                {
                    shell->cache_buff[i] = shell->cache_buff[i - 1];
                }
            }
            shell->cache_buff[shell->cursor_position] = data[0];
            shell->cache_size++;
            shell->cursor_position++;
            SHELL_INSTE_STR(shell, &data[0]);
            return 0;
        }
        else return -SHELL_OUT_OF_CACH;
        break;
    }

    /*now checke command*/
    /*get buff*/
    if(shell->cache_size > 1)
    {
        //archive cache to histiory
        if (Shell_Archive_History(shell) < 0)
        {
            SHELL_ERR(shell, "cache history is loop or memory out");
        }

        /*this function will change cache, so must archive cache befor this function*/
        //dilime params
        void *params[SHELL_MAX_PARAM_NUMBER + 1];
        params[0] = (void *)shell;
        short params_number = Shell_Get_Param(shell->cache_buff, " ", &params[1], SHELL_MAX_PARAM_NUMBER);
        if (params_number < 1) return -SHELL_NO_FETCH;
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
    }
    /*clean buff*/
    SHELL_CLEAN_BUFF(shell);
    SHELL_CLEAN_CURSOR(shell);
    //anthe loop steart
    shell->history_entry = 0;
    SHELL_HEAD(shell, shell->ret, ANSI_NONE);
    return 0;
}
