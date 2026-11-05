/*
 * @brief 
 *
 * Custom MNXYG 
 * 
*/

#if !defined(MICRO_SHELL_H)
#define MICRO_SHELL_H 

#ifdef __cplusplus
extern"C"{
#endif

#include "./fifo/fifo.h"
#include "./linked-list/linked-list.h"

/********************************SETTINGS************************************** */

/*Max History cache size*/
#define SHELL_CACHE_HISTORY         256 /*bytes*/

/*Max Cache buff size*/
#define SHELL_CACHE_DEEP            64  /*bytes*/

/*Max cache or output buff*/
#define SHELL_POP_CACHE_DEEP        4096 /*bytes*/

/*Max param num*/
#define SHELL_MAX_PARAM_NUMBER      6

/*Host name max len*/
#define SHELL_MAX_HOST_NAME_LEN     16

/*Host name default*/
#define SHELL_HOST_NAME "root"


/**********************************DEFINE*************************************** */

#define    SHELL_ERROR      1
#define    SHELL_BUSY       2
#define    SHELL_OVER_RANGE 4
#define    SHELL_NO_FETCH   5
#define    SHELL_NO_MEM     6
#define SHELL_OUT_OF_CACH   7
#define SHELL_COMMAND_EXITS 8

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

#if !defined(UNUSED)
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif // UNUSED   

typedef void Shell_Send_Data_t(char*, unsigned short);

typedef struct
{
    /*it is fifo for get data*/
    struct FIFO_buff_t fifo;

    /*this use for storage history entry*/
    unsigned int history_entry;

    /*this use for how many size of history can use*/
    unsigned int history_size_limit;

    /*this use for flag how many size history used*/
    unsigned int history_size;

    /*this linked list use for storage history*/
    Linked_List_Node_t *history;

    /*input table size*/
    unsigned int cache_size;

    /*input table position*/
    unsigned int cursor_position;

    /*retun value save*/
    int ret;

    /*this is linked list for shell command, it could use for*/
    /*register command form external function and file, it usefor*/
    /*for uncrach this lib to orther files*/
    Linked_List_Node_t *Shell_command;

    /*it is send byte to out side api function*/
    Shell_Send_Data_t *terminal_send_message;

    /*input table cache*/
    char cache_buff[SHELL_CACHE_DEEP];

    /*host name*/
    char host_name[SHELL_MAX_HOST_NAME_LEN];

    /*use for mulit key like direction key and esc or ctrl key*/
    unsigned char mulit_key_status;

    /*how many registed command*/
    unsigned char command_num;

}Shell_Handle_t;

/*the param struct*/
typedef struct  
{
    /*orther  full name like --help*/
    char *full_name;

    /*orther nickname like -h*/
    char *nick_name;

    /*help strings of param*/
    char *help;

}Shell_Param_Typedef;

typedef int Terminal_Function_t(int,void**);

//command typedef
typedef struct 
{
    /*command of params, is will define by const and head pointer*/
    const Shell_Param_Typedef *params;

    /*command function, the first arg is call this function`s shell,*/
    /*like set 1 2 3 have 5 args the first arg is shell`s pointer*/
    /*the secned arg si "set", and 3rd arg is "1", all char type*/
    /*unlise shell is pointer type, more detil look at README or shell.c`s default function*/
    /*so, can use first to change Shell_Hanlde_t * to get shell priner*/
    Terminal_Function_t *terminal_function;

    /*command name, it must be static const strings*/
    char *name;

    /*param number*/
    unsigned char param_num;

}Shell_Command_Typedef;

/******************************************HELPER FUNCTION************************************************* */

/*micor magic*/

/*
 * @brief 该宏用于定义一个参数子项，
 *          这是一个超级宏的内部子项，
 *          不可单独使用。
 * @param na 参数全名，类似 --help
 * @param ni 参数别名，类似 -h
 * @param he 参数帮助文本
 * @note 由于有潜在的"-"字符，所以上述三个参数必须为字符串值
*/
#define SHELL_PARAM_DEFINE(na, ni, he) \
{\
    .full_name = (na),\
    .nick_name = (ni),\
    .help = (he),\
},

/*
 * @brief 该宏用于定义一个命令，可以包含任意多
 *          的参数子项，这个宏的写法可能会有些臃肿，
 *          建议先阅读一下README.MD以获取更多信息
 * @param na 函数名称，shell识别的函数名称，不需要加双引号
 * @param he 函数帮助，显示在help命令或者调用参数 --help -h，
 *          注意，需要用户实现
 * @param api 用户函数，终端输入函数名称(na)后，就会调用这个
 *          函数，参数解析以及运行对应的二级函数都需要用户自行
 *          编写完成，但库内提供了一些辅助函数，希望能减少用户的
 *          工作
 * @note  主干命令的help参数被默认放置在后缀 --help
 *          以及-h 的参数下，请注意不要让这两个参数被占用
*/
#define SHELL_COMMAND_DEFINE(na, he, api, ...)                          \
const Shell_Param_Typedef PARAM_##na [] = {                             \
    SHELL_PARAM_DEFINE("--help", "-h", (he))                            \
    __VA_ARGS__                                                         \
};                                                                      \
const Shell_Command_Typedef COMMAND_##na = {                            \
    .name = #na,                                                        \
    .params = PARAM_##na,                                               \
    .terminal_function = (api),                                         \
    .param_num = sizeof(PARAM_##na)/sizeof(Shell_Param_Typedef),        \
};                                                                      \
LINKED_LIST_NODE_DEFINE_STATIC((na),                                    \
    (void *)&COMMAND_##na,                                              \
    sizeof(COMMAND_##na));

/*this is the simple example to how builed a commad struct*/
// SHELL_COMMAND_DEFINE(cmd, "hello is cmd command help", NULL,    
//     SHELL_PARAM_DEFINE("--shutdown", "-s", "is for shut down system")
//     SHELL_PARAM_DEFINE("--reboot", "-r", "is for reboot system")
//     /*you can use infenity number of SHELL_PARAM_DEFINE to define params*/
// )

/*
 * @brief 该宏会使终端输出另起新行
*/
#define SHELL_NEW_LINE(x)           Terminal_Print((x),"\r\n")

/*
 * @brief 该宏会让终端上的输出完全清除
*/
#define SHELL_CLEAN_SCREEN(x)       Terminal_Print((x),"\033[H\033[2J")

/*
 * @brief 这三个宏是帮助宏，使用方式和Terminal_Printer一致，但是会有对应的颜色显示，并且有错误指示前标
*/
#define SHELL_INF(x,...)            Terminal_Print((x),ANSI_COLOR_GRN "<inf> : %s\r\n" ANSI_COLOR_RESET, ##__VA_ARGS__)
#define SHELL_WRN(x,...)            Terminal_Print((x),ANSI_COLOR_YEL "<wrn> : %s\r\n" ANSI_COLOR_RESET, ##__VA_ARGS__)
#define SHELL_ERR(x,...)            Terminal_Print((x),ANSI_COLOR_RED "<err> : %s\r\n" ANSI_COLOR_RESET, ##__VA_ARGS__)

/*function*/

/*
 * @brief 该函数用于回传命令列表的大小，
 *         也就是命令的数量
 * @param shell
 * @return number of command
*/
unsigned char Shell_get_Command_List_Size(Shell_Handle_t *shell);

/*
 * @brief 该函数用于遍历命令链表，并且返回命令列表
 * @param shell
 * @param command_list
 * @return 0 is success
 * @return - is error
*/
int Shell_Foreach_Command(Shell_Handle_t *shell, Shell_Command_Typedef *command_list[]);

/*
 * @brief 该函数用于遍历命令链表，搜索对应名称的命令，并将命令结构返回
 * @param shell
 * @param name 命令名称的字符串
 * @return 命令结构的指针或者NULL
*/
const Shell_Command_Typedef* Shell_Serch_Command(Shell_Handle_t *shell, char* name);

/*
 * @brief 该函数用于获取参数的顺序值，用于匹配次级参数
 * @param cmd 该命令结构体的指针
 * @param param 该命令参数的字符串
 * @return 参数的顺序值，从0开始，0为定义的第一个参数
 * @return - is error
 * @note 该函数为用户可以直接在自定义命令里使用的辅助函数，通过判断返回的值，即可知道传入的次级参数是什么
*/
int Shell_Complite_Param_To_Num(const Shell_Command_Typedef *cmd, char *param);

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
 *                  --help      -h      use for display help informations
 *                  --load      -l      load settings
 *                  --save      -s      save settings
*/
int Shell_Display_Command_Help (Shell_Handle_t *shell, const Shell_Command_Typedef *cmd);

/*
 * @brief 该函数用于将输入的参数转为整型
 * @param param
 * @param num
 * @return 0 is success
 * @return nagetive is error code
*/
int Shell_Conversion_Str_to_Int (char *param, int *num);

/*
 * @brief 该函数用于将输入参数转换为浮点型
 * @param param
 * @param num
 * @return 0 is success
 * @return nagetive is error code
*/
int Shell_Conversion_Str_to_Float (char *param, float *num);

/*
 * @brief 该函数用于将输入参数转为bool型
 * @param param
 * @param true_type 判断为true的字符串
 * @param false_type 判断为false的字符串
 * @return 0 false
 * @return 1 true
 * @return nagative is error code
*/
int Shell_Conversion_Str_to_Bool (char *param, const char *true_type, const char *false_type);

/******************************************API FUNCTION********************************************* */

/*
 * @brief 该函数初始化Shell层，Shell可以在没有foc实例的状态下独立运行
 * @param shell 终端的虚拟句柄
 * @param terminal_send_message 发送API函数，需要用户实现，用于发送字符串
 * @return FOC_OK
 * @return -FOC_ERROR   初始化失败
*/
int Shell_Iint (Shell_Handle_t *shell, void (*send_data)(char *message,unsigned short size));

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
int Shell_Register_Command(Shell_Handle_t *shell, Linked_List_Node_t *command);

/*
 * @brief 该函数将数据推入内部环形缓冲区，处理函数会在非中断条件下处理这些数据
 * @param shell 终端的虚拟句柄
 * @param data 数据头指针
 * @param size 数据长度
 * @return FOC_OK
 * @return -FOC_OVER_RANGE  超出缓冲区长度
*/
int Shell_push_data(Shell_Handle_t *shell, char *data, unsigned char size);

/*
 * @brief 该函数用于循环运行shell解算和命令处理，每调用一次理论上最多能处理一半32字节的数据
 *       该函数处理后会自动退出，如果需要在操作系统环境下将该函数当作线程运行的话，需要额外的
 *      死循环
 * @param shell 终端的虚拟句柄
 * @return FOC_OK
 * @return -FOC_BUSY 缓冲区是空的，没有东西需要处理
 * @return -FOC_ERROR 输入的命令触发的错误
*/
int Shell_Task (Shell_Handle_t *shell);

/*
 * @brief 该函数用于向终端发送格式化字符串在，实际使用中
 *          这个函数将完全替代printf的功能
*/
void Terminal_Print (Shell_Handle_t *shell, const char* format,...);

#ifdef __cplusplus
}
#endif
#endif //MICRO_SHELL_H 
