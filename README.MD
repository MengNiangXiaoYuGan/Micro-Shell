
# <font color=#e294cb>*Micro Shell*</font> -> 一款面对分层架构和单片机进行优化的Shell驱动库

## 简介
*Micro Shell*基于`GNU C99`标准编写，是一个具有简单功能的可以方便移植的多实例shell库，具有以下亮点
  1. 上下层完全解耦，可以很方便地移植到其他地方，具体看[移植部分](#移植)
  2. 仅依赖`fifo`以及`linked-list`两个组件
  3. 支持动态增加用户命令，通过构建命令宏，用户的命令解算可以通过几个辅助函数进行状态机选择,具体看[开始使用](#使用)
  4. 支持历史记录，使用统一的字节数限制历史记录，防止占用过多堆栈（历史记录使用malloc进行内存分配，之后需要修改）
  5. 支持单独编译，不需要修改c文件即可与其他组件一起使用
  6. 支持tab补全(功能仍需完善)
  7. 支持返回值显示

## 开始
1. 风水宝地，使用`git clone https://github.com/MengNiangXiaoYuGan/Micro-Shell.git`,将该库拉到指定位置
2. 将`Micro-Shell`文件夹下的头文件和源文件加入项目的编译路径以及编译文件中
3. 在[设置文件](./micro-shell.h)中查看用于设置的宏定义，可以用一个额外的宏定义替换默认设置，以达到外部传入设置的目的

## 移植
为了移植该库，需要完成一个API函数，并且对接一个API函数

|函数名|用途|
|:----:|:----|
|`Shell_Send_Data_t *terminal_send_message`|该函数用于向外部发送字节，通过将这个函数传入的内容包装发送给终端或者其他什么东西，就能实现库与人机界面的通讯|
|`Shell_push_data`|该函数用于向Shell的缓冲区内推入数据，这些数据会在Shell的Task函数内被取出处理，从而实现回显，命令解析，运行命令等各种各样的功能|

  - `Terminal_Function_t *terminal_function`本质上是一个函数指针，它的原型是
~~~~~ C
  typedef void Shell_Send_Data_t(char*, unsigned short);
~~~~~
也就是说，用户需要实现一个类似`void shell_send_message_api (char *data, unsigned short size)`这样的函数，然后在初始化的时候传入Shell_Init中，并且在数据传入的时候调用`Shell_push_data`将数据压入缓冲区等待处理，下面这个例子能很好理解如何初始化一个shell,并且给shell发送信息，让shell运行起来
~~~~~ c
void shell_send_message_api (char *data, unsigned short size)
{
  /*your send message code like this*/
  char send[size];
  memcpy(send, data, size);
  /*put data to terminal*/
  puts(send);
}

/*if you want to use shell in orther function*/
Shell_Handle_t shell;

int main ()
{
  if(Shell_Init (&shell, shell_send_message_api))
  {
    /*error, cannot init shell*/
    return -1;
  }
  while(1)
  {
    /*init your buff, size is not must be 10, you can use orther
     number or use interruput function to push data to shell ring
     buff*/
    char get[10];
    /*get data form human input like key bord or master app*/
    gets(get);
    /*push data to ring buff*/
    Shell_push_data(&shell, get, sizeof(get));

    /*call task function to precessing data*/

    /*If you input a large amount of data at 
    once and fetch and process the data in the
     same loop, you might need to use multiple 
     processing loops to finish handling the data.*/
    while (Shell_Task(&shell) != 0);
  }
  return -1;
}
~~~~~
 - 需要注意的是，在上述实例中，每输入十个字符才会进行一次回显，效果其实不是很好，这里只是展示该库的多字符处理能力，如果使用阻塞式获取字符，轮询处理的话，建议将主循环的函数简化成下方所示的状态
~~~~~c
while(1)
{
  /*use single char data buff*/
  char get;
  /*get data form human input like key bord or master app*/
  gets(&get);
  /*push data to ring buff*/
  Shell_push_data(&shell, %get, sizeof(get));

  /*call task function to precessing data*/
  Shell_Task(&shell);
}
~~~~~
 - 上述做法最大的好处就是输入就会立刻回显，但在应付方向键这种多字符的按键时，有时候会因为设备本身的原因丢失内容，从而回显出奇怪的符号，例如`[A`(上方向键)和`[B`下方向键。如果需要更好的处理方式的话，建议考虑使用空闲中断(如有)获取数据，轮询处理数据。`Shell_Push_data()`函数仅包含将数据压入FIFO的过程，并不会对数据进行处理，运行时间很短，可以直接在中断中调用，不需要额外的FIFO进行中转。

## 使用
*Micor Shell*内置四个命令如下

| 命令 | 用途 |
|:-----:|:-----|
|`test`|显示输入的命令以及参数，用以测试Shell数据收发正常|
|`help`|显示命令列表，以及命令的帮助|
|`clean`|清除屏幕上所有显示的东西|
|`clear`|与上一个命令一样|

当然，相信各位一定不是为了使用这四个简单到不行的命令而移植这个库的，所以，使用的第一步，就是开始加载自己的函数，作为一个命令启动，*Micro Shell*的独特性就在于此，该库弃用了静态结构体数组命令表，转而使用链表作为命令索引，所以用户可以在初始化后，在库的外部动态注册命令并且使用，当然，为了实现这个特性，*Micor Shell*使用了了大量的指针，对内存的占用以及flash的占用会比较大，在当前版本，静态编译的库占用的空间大概在`RAM:0.8Kbytes Flash:15Kbytes`左右，随着命令的增加，占用会进一步增加，为了安全考虑，所有的命令结构都使用静态常量声明，而命令结构的链表索引使用静态声明，而不是通过malloc分配，在当前版本中，只有历史记录索引以及历史记录本身使用malloc分配的内存。随着命令数量的增长，消耗flash的速度应该会比消耗ram的速度要快不少。

为了加载自己的函数，用户需要先声明定义一个用户函数，之后使用简单的*宏魔法*将用户函数以及对应的命令信息定义为一个静态的结构体，之后通过调用注册函数，将结构体指针纳入`Shell`实例的“搜索路径中”，从而实现在`micor-shell.c`源文件外加载命令的过程。由于原理讲起来比较复杂，首先我们先简单声明一个例子，之后再慢慢解析:
~~~~~c
/*
 * @brief this function use for switch led on or off
 * @param 
 * @return 0 is success
 * @retrn -1 is error IO
 * @retun 2 is led aleady on or off
 */
int cmd_led (int argc, void *argv[])
{
  static unsigned char led_status;
  /*get shell handle*/
  Shell_Handle_t *shell = (Shell_Handle_t *)argv[0];
  /*get this command steurct*/
  /*can use while or for to foreach all param grup*/
  const Shell_Command_Typedef *cmd = Shell_Serch_Command(shell, argv[1]);
  /*get param list and num*/
  int param_num = Shell_Complite_Param_To_Num(cmd, argv[2]);
  switch(param_num)
  {
    case 0:
      /*the number 0 param target must is help*/
      /*call *cmd* --help or *cmd* -h to trigger it*/
      /*can put any wont to display function of help on there*/
      Shell_Display_Command_Help(shell, cmd);
      break;
    case 1:
      if (led_status != 0)
      {
        SHELL_WRN(shell, "led all ready on");
        return 2;
      }
      if (*set_led_on()*) return -1;
      led_status = 1;
      break;
    case 2:
      if (led_status == 0)
      {
        SHELL_WRN(shell, "led all ready off");
        return 2;
      }
      if (*set_led_off()*) return -1;
      led_status = 0;
      break;
    default:
      /*reurn is negtive value*/
      /*there are no any param to get*/
      Terminal_Print(shell,"led is %s\r\n", led_status == 0 ? "off" : "on");
      break;
  }
  return 0;
}

/*Build command structure*/
SHELL_COMMAND_DEFINE(led, 
    "is user function use for turn led on or off", 
    cmd_led,
    SHELL_PARAM_DEFINE("on", "-o", "turn on led")
    SHELL_PARAM_DEFINE("off", "-f", "turn off led")
)
~~~~~
  接下来，开始解析上述行为到底做了什么

  1. 首先，用户需要声明定义一个函数，这个函数的原型需要与下方函数类型匹配：
~~~~~c
typedef int Terminal_Function_t(int,void**);
~~~~~
 -  这个函数就是命令的api，当用户在Shell中敲下这个命令的时候，*Micro Shell*就会调用这个函数，当然，函数名和命令名可不太一样

  2. 这个时候，其实用户还不知道在函数里怎么处理传入的各种奇奇怪怪的参数，所以，定义声明完成后，先不用急着实现函数内容，先到函数下方，进行一个结构体的声的明：
- 使用SHELL_COMMAND_DEFINE宏声明一个命令结构体，该宏的原型如下
~~~~~c
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
~~~~~
- 看着很长，但其实很好理解，第一个输入的参数是命令名，也是这个链表节点的名称，在Shell中想要调用这个函数，就需要输入 `led` 而不是 `cmd_led`，第二个参数是帮助字符，当用户调用`help`命令的时候，这个字符就会格式化地输出到界面上，同时这个宏还定义了一个全名为`--help`,别名为`-h`的参数，当用户调用`led --help`或者`led -h`的时候，就会返回默认的参数值，这点我们之后讲，第三个参数就是用户之前声明定义的函数，作为命令的api被传入，由于这个宏是一个可变长度参数宏，我们还可以在默认的参数外继续添加任意数量的参数，这时候就需要使用到`SHELL_PARAM_DEFINE`宏，这个宏的原型如下
~~~~~c
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
~~~~~ 
- 这个宏就比上一个短多了，这个宏会声明一个结构体元素，添加在命令的参数结构体内，以`--help`参数为例，实际的写法大概是这样的
~~~~~c
SHELL_PARAM_DEFINE("--help", "-h", "this is the help of led")
~~~~~
 - 由于`help`参数已经被命令定义占用了，所以实际上我们并不需要声明任何有关`--help`或者`-h`的参数，在这个例子中，可以看到，除了默认的`--help`参数外，例子还声明了两个参数，一个是`on`,一个是`off`，请注意它们声明的顺序，声明的顺序对应了使用辅助函数时返回的顺序值，所以顺序很重要，在声明完命令后，一定一定不要修改参数的顺序，如果需要添加额外的参数，尽量在最后一个参数的后方添加，这样对代码的扰动是最小的，并且在使用辅助函数进行参数解析的时候，随意排布的参数并不会对实际使用造成影响（可能会有一点，帮助界面的参数排布可能不是很拟人）

- 接下来，我们终于可以回到命令的api函数，进行函数实现。

 3. 要实现函数内容，我们需要知道*Micro Shell*对标准C函数输入参数的一点小小的修改

 - 在标准C函数中，输入的参数个数和参数解析一般如下
~~~~~c
  /*shell input*/
  cmd 1 2 3 4

  /*param list*/
  argc = 5
  argv = {{cmd}，{1}，{2}，{3}，{4}}
~~~~~
- 但是在*Micor Shell*库中，API函数传入的参数不太一样，区别如下
   - 1. *Micro Shell*的参数数量`argc`与标准C一致。
   - 2. *Micro Shell*的参数列表比标准C多一个元素，用来放置Shell句柄`Shell_Handle_t`
   - 所以在*Micro Shell*中，同样的命令，传入命令API函数的参数列表会变成这样
~~~~~c
  /*param list*/
  argc = 5
  argv = {{&shell}, {cmd}, {1}, {2}, {3}, {4}}
~~~~~
 - 于是我们就可以在API函数中通过`Shell_Handle_t *shell = (Shell_Handle_t *)argv[0];`将shell句柄强转出来，以方便后续的使用，同时这样做也有一个好处，可以在多实例shell的情况下调用一个共同的命令api，但是需要注意的是，上述例子中，命令API的静态变量声明`static unsigned char led_status`在多实例的状态下很有可能出现问题，如果真正需要多实例的情况，请自行修改。

 - 在这之后，我们就需要获取参数列表，与传入的参数进行比对，从而触发我们想要的功能。但是在获取参数列表之前，我们还需要获取当前的命令结构体，因为参数列表是存储在命令结构体里面的

> [!IMPORTANT]
> 可能这个时候会有用户在想，我明明声明的结构体都是静态的，为什么不直接调用结构体的名称获取对应的资源呢？
> 值得注意的是，这样做是不行的，因为声明的结构体位置在函数的后面，除非将函数的实现挪到`SHELL_COMMAND_DEFINE`
> 宏定义后面，否则是无法在本命令api函数实现里面直接调用本命令api的参数以及命令结构体的

 - 那么，要怎么做呢？很简单，命令API传入的参数组中就有本命令的名称，调用函数`Shell_Command_Typedef *cmd = Shell_Serch_Command(shell, argv[1]);`将命令名称与命令表进行比对，就能获取自身命令的命令结构体指针了。

> [!NOTE]
> 值得注意的是，在声明命令及参数结构体的时候，已经将其声明为常量，所以在这里所使用的指针，也最好是常量指针

 - 获取到该命令结构体后，用户就可以使用一大串的字符串比对函数进行参数比对，然后用一大串`switch`函数进行功能实现，是不是觉得很麻烦？没错，所以*Micor Shell*为用户准备了另一个辅助函数`Shell_Complite_Param_To_Num` 其函数原型如下
~~~~~c
/*
 * @brief 该函数用于获取参数的顺序值，用于匹配次级参数
 * @param cmd 该命令结构体的指针
 * @param param 该命令参数的字符串
 * @return 参数的顺序值，从0开始，0为定义的第一个参数
 * @return - is error
 * @note 该函数为用户可以直接在自定义命令里使用的辅助函数，通过判断返回的值，即可知道传入的次级参数是什么
*/
int Shell_Complite_Param_To_Num(Shell_Command_Typedef *cmd, char *param);
~~~~~
 - 该函数可以将传入的字符串与命令结构体中的参数列表进行比对，然后返回命令的序号，这就是为什么在声明命令参数的时候，参数的顺序如此重要，并且在之前的篇幅中，我们也介绍过，0号参数默认为`--help`或者`-h`，而从1开始，根据在`SHELL_COMMAND_DEFINE`宏中声明的参数顺序，一一递增，在上面的例子中，`--on`参数的顺序值是1，`--off`参数的顺序值是2，用户可以很方便地调用一个`switch`函数实现一次参数解析。

 - 那么多个参数怎么办呢？我们比较推荐的方法是使用`for`循环，依次调用`Shell_Complite_Param_To_Num`函数进行参数解析，如果参数后面跟有数值的话，则可以使用库内预留的函数进行浮点数，整数，字符串布尔值的转换。随后将参数继续往后推进，直到遍历`size`个参数。当前支持的参数转换函数如下所示
  
|函数名称|函数用途|函数参数|函数返回值|
|:-----|:-----|:-----|:-----|
|`Shell_Conversion_Str_to_Int`|将传入参数转换为整型|`param`:输入的参数字符串，yi'\0'结尾 `num`：回传数字的地址，解析的数字会回传到这个地址的变量下|`0`代表转换成功，`-5`为参数里面含有无法转换的东西，`-1`为转换失败，参数不合法|
|`Shell_Conversion_Str_to_Float`|将传入参数转换为浮点型|`param`:输入的参数字符串，yi'\0'结尾 `num`：回传数字的地址，解析的数字会回传到这个地址的变量下|`0`代表转换成功，`-5`为参数里面含有无法转换的东西，`-1`为转换失败，参数不合法|
|`Shell_Conversion_Str_to_Bool`|将传入参数转换为布尔值|`param`:输入的参数字符串，yi'\0'结尾 `true_type`：当参数字符串与这个这个字符串匹配，则返回true `false_type`:当参数字符串与这个字符串匹配，则返回false|`0`代表false，`1`代表true， `-1`代表两者皆不匹配|

  4. 最后，在`Shell_Init`函数之后的随便哪个地方调用`Shell_Register_Command`函数，将命令注册到Shell实例里，即可正常使用。
~~~~~c
  Shell_Register_Command(shell, &led);
~~~~~

> [!CAUTION]
> `Shell_Register_Command`函数必须在`Shell_Init`函数之后调用，否则有可能出现链表回环的情况。

  5. 之后就可以快乐使用了

## 深入了解
 - 这个组件原本是MNXYG-FOC库的一个小组件，但是在优化的过程中，渐渐形成了一个独立的解耦库，并且在体验了[`Zephyr RTOS`](https://docs.zephyrproject.org/latest/index.html)的shell子系统后，我认为这个组件已经优化到足够独立出来，作为一个独立的组件或者库被使用。
 - 关于自动补全的机制
   1. 由于没有时间控制函数，所有的自动补全命令由`tab`键单击完成
   2. 在没有输入的情况下，单击`tab`键，将会显示所有命令，但不会显示任何帮助
   3. 在有部分输入的情况下，单击`tab`键，将会根据当前输入，在下方列出候选项，如果没有候选项，则只会列出一个空行
   4. 在输入了一个完整命令的情况下，单击`tab`键，将会显示当前命令的标准帮助文档，也就是调用显示帮助的用户辅助函数
   5. 在输入一个完整命令并且输入了部分参数的情况下，单击`tab`键，将会显示候选的参数选项以及参数对应的帮助文本，这个机制无论有几个参数都会生效，生效的范围为最后一个参数
  
## 更新记录
 * 2026年11月5日 1.0.0发布
 * 2026年11月6日 1.0.1 安全更新，修复了一些会导致崩溃的bug
 * 2026年11月8日 1.0.2 紧急安全更新，修复了几个会导致系统完全不可用的bug
 * 2026年11月10日 none 热修，修复了一些初始化bug