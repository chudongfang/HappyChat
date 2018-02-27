# HappyChat/开心聊
    The ChatRoom based on Linux/Epoll /  一个基于Linux平台的聊天室

## 项目作者：  楚东方    

 

### 1. 引言

#### 1.1 项目综述

      该项目为聊天室，主要为了实现聊天，文件传输，方便linux环境下的，交流与聊天。

实现功能：

- 1.好友管理：

      (1)添加好友

      (2)删除好友

- 2.群管理

      (1)创建群

      (2)加群

      (3)退群

      (4)解散群

- 3.文件传送

      实现了上传和下载的断点续传

- 4.聊天界面分屏

      利用光标的移动对输入和屏幕聊天记录输出的分屏

- 5.MD5加密 
  
      利用MD5算法，对用户的密码进行加密，对原来的MD5加密进稍微的改动，把字符串类型加密到unsigned int 类型

- 6.离线传输 

      对私聊,群聊，传送文件都添加了离线功能，对方上线即时传送

- 7.即时状态显示

      好友列表会及时更新，并用彩色显示当前好友的状态

- 8.数据库

      利用mysql C语言的API函数，应用mysql数据库对聊天记录进行存储，保证聊天记录的有效性

- 9.多路复用

      epoll实现服务端的多路复用，保证服务端连接客户端数据的稳定性

- 10.容错处理

      对于输入内容进行判别，防止在不正规的操作下程序崩溃。

 


 

#### 1.2 项目开发环境

开发环境：Ubuntu 16.04 STL  6   gcc编译器

程序运行环境：

- 1.局域网/外网

- 2.安装mysql数据库        sudo apt-get install mysql-server mysql-client

- 3.安装mysql数据库开发包  sudo apt-get instal libmysqlclient-dev

- 4.数据库   
        
        Database   :happychat  

        Table         :message_tbl     

        表中变量:recv_name varchar(1024)    send_name varchar(1024)  mes varchar(2048)

---
 
### 2. 数据结构说明

    本程序主要采用数组做为数据结构，包括数组元素的删除，添加，遍历。
    数组作为一种线性结构，以包的形式进行定义。


---

### 3. 模块设计

#### 3.1 程序函数调用图及模块化分

##### Client总图：
![Client总图](http://img.blog.csdn.net/20160905205507825?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 

##### 模块划分图：

![模块划分图](http://img.blog.csdn.net/20160905205524313?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

![模块划分图](http://img.blog.csdn.net/20160905205603935?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 

##### Server总图：

![Server总图](http://img.blog.csdn.net/20160905205616826?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 

 

##### 模块划分：
![模块划分](http://img.blog.csdn.net/20160905205627544?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 
![模块划分](http://img.blog.csdn.net/20160905205639794?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 

#### 3.2 功能设计说明

##### 3.2.1 模块1

###### 功能描述

实现文件的断点发送和断点接受

###### 算法和流程图

![算法和流程图](http://img.blog.csdn.net/20160905205721785?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 
![算法和流程图](http://img.blog.csdn.net/20160905205844295?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
 

###### 函数说明


发送文件断点续传：

1.
```
void send_file()  

功能：       向服务端请求发送文件，并将文件的大小发送给服务端

参数：       无

返回值：     无

算法说明：   对文件大小进行字符串解析
```
2.
```

void file_recv_begin(PACK *recv_pack)

功能：       * 跟据收到包的内容，客户端请求发送文件

 * 首先判断是否已经有该文件信息

 * 如果有，返回该文件的大小

 * 否则，建立文件信息，并返回文件大小为0

参数：       接送到的包指针

返回值：     无

算法说明：   对文件大小进行字符串解析，添加数组元素
```


3.
```

void * pthread_send_file(void *mes_t)

功能：       当接收到允许发送的消息时，开启线程发送文件

参数：       结构体指针

返回值：     无

算法说明：   对文件大小进行字符串解析

 

 

void send_file_send(int begin_location,char *file_path)

功能：       从起始位置向服务端发送文件

        参数：       int begin_location  ：文件起始发送位置

      char *file_path ：文件名

返回值：     无

算法说明：   打开文件，while循环发送文件

 ```

4.

```
void *file_send_send(void *file_send_begin_t)

功能：       在发送过程中，不断监测接收端的状态

  * 若接收端下线，则向接收端发送提醒消息

参数：       结构体指针

返回值：     无

算法说明：   对文件大小进行字符串解析，while发送文件，并不断检测发送者 信息

 ```

5.
```
void mes_sendfile_fail(int id)  

功能：      处理文件上传失败，并询问是否重发，进行断点续传

参数：       消息数组中消息ID

返回值：     无

算法说明：   询问是否重发

 

void * pthread_send_file(void *mes_t)

功能：       当接收到允许发送的消息时，开启线程发送文件

参数：       结构体指针

返回值：     无

算法说明：   对文件大小进行字符串解析

 

 

void send_file_send(int begin_location,char *file_path)

功能：       从起始位置向服务端发送文件

        参数：       int begin_location  ：文件起始发送位置

      char *file_path ：文件名

返回值：     无

算法说明：   打开文件，while循环发送文件
```
 

 

接受文件断点续传：

1.
```
void *pthread_check_file(void *arg)  

功能：       不断检测文件状态，当文件传输失败，发送提醒给客户端

                     当文件传输成功时，发送提醒给客户端接受

                     当文件传输完成，删除服务端缓存文件，和对应文件数组信息

参数：       无

返回值：     无

算法说明：   while循环不断遍历  对文件大小进行字符串解析
```
 

2.
```
void mes_recv_requir(int id) 

功能：       处理下载文件请求，询问是否接收文件，发送信息给服务端，开启 线程写文件

参数：       消息数组中消息ID

返回值：     无

算法说明：   对文件大小进行字符串解析 询问是否接受文件 开启线程写文件

 

 

 

void *pthread_recv_file(void *par_t) 

功能：       接收文件线程，从存储接收包的地方检索到信息

 并写入文件，当文件写入完成，向服务端发送信息，关闭线程

参数：       无

返回值：     无

算法说明：   while循环写文件

```

3.

```
void file_send_begin(PACK *recv_pack) 

功能：       客户端请求接收文件，解析出客户端已经

             接收的文件字节数，然后从该字节数开始发送

参数：       接受的包的指针

返回值：     无

算法说明：   对文件大小进行字符串解析

 

void *file_send_send(void *file_send_begin_t) 

功能：        * 根据文件的起始位置，开始发送，

参数：       结构体指针

返回值：     无

算法说明：   对文件大小进行字符串解析   while循环发送文件

```

 

4.
```
void *pthread_check_file(void *arg)  

功能：       不断检测文件状态，当文件传输失败，发送提醒给客户端

                     当文件传输成功时，发送提醒给客户端接受

                     当文件传输完成，删除服务端缓存文件，和对应文件数组信息

参数：       无

返回值：     无

算法说明：   while循环不断遍历  对文件大小进行字符串解析
```
 

5.
```
void mes_recvfile_fail(int id)

功能：      处理接收文件中断信息，并询问是否继续接收

参数：       消息数组中消息ID

返回值：     无

算法说明：   对文件大小进行字符串解析 询问是否继续接受文件

                     开启线程写文件
```
 

6.
```
void file_send_begin(PACK *recv_pack) 

功能：       客户端请求接收文件，解析出客户端已经

             接收的文件字节数，然后从该字节数开始发送

参数：       接受的包的指针

返回值：     无

算法说明：   对文件大小进行字符串解析

 

void *file_send_send(void *file_send_begin_t) 

功能：        * 根据文件的起始位置，开始发送，

参数：       结构体指针

返回值：     无

算法说明：   对文件大小进行字符串解析   while循环发送文件
```
 

7.
```
void *pthread_recv_file(void *par_t) 

功能：       接收文件线程，从存储接收包的地方检索到信息

 并写入文件，当文件写入完成，向服务端发送信息，关闭线程

参数：       无

返回值：     无

算法说明：   while循环写文件

 

 

 

void file_send_finish(PACK *recv_pack)

功能：       * 客户端接收完，会发送确认信息，

 * 然后把文件状态改为:已经发送完毕

  * 其他函数对该文件进行相应处理

参数：       接受包的指针

返回值：     无

算法说明：   改变文件状态

 

 

void *pthread_check_file(void *arg)  

功能：       不断检测文件状态，当文件传输失败，发送提醒给客户端

                     当文件传输成功时，发送提醒给客户端接受

                     当文件传输完成，删除服务端缓存文件，和对应文件数组信息

参数：       无

返回值：     无

算法说明：   while循环不断遍历  对文件大小进行字符串解析
```

 


##### 3.2.2 模块2

######  功能描述


    对聊天界面进行优化，在聊天时保证了信息流动显示，输入与显示分屏，

    提升界面友好性

###### 算法和流程图

![算法和流程图](http://img.blog.csdn.net/20160905205857795?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)


###### 函数说明


1.
```
void send_mes_to_one()

功能：       私聊，开启线程显示信息，发送信息到服务端

参数：       无

返回值：     无

算法说明：   无

 

void send_mes_to_group()

功能：       群聊，开启线程显示信息，发送信息到服务端

参数：       无

返回值：     无

算法说明：   无

 

void *show_mes(void *username)

功能：       在聊天的同时启动,启动线程读取，存储区域的

  消息，并显示出来

参数：       消息发送者姓名

返回值：     无

算法说明：   遍历数组

 

void print_mes(int id)

功能：       根据寻找到的包把信息输出

参数：       包数组ID

返回值：     无

算法说明：   读取数组元素

 

void show_mes_smart(char *name  ,char *mes) 

功能：       分屏显示聊天信息

参数：       char *name：信息发送者姓名

     char *mes：信息内容

全局变量:    m_print_mes[] 缓存数组

     m_print_mes_num 缓存数组中消息的个数

返回值：     无

算法说明：   更新缓存数组：数组依次往后覆盖，把数组第一个元素覆盖，把留  出的最后一个数组位置给新的信息

     利用光标显示缓存数组信息

程序简略代码：

void show_mes_smart(char *name  ,char *mes) //该程序连续显示6条信息

{

number = 6;

  if(m_print_mes_num == number) {        //如果缓存数已满，覆盖第一个元素，更   新最后一个元素

        for(int i=1;i<=5 ;i++)

            m_print_mes[i] = m_print_mes[i+1];

     strcpy(m_print_mes[m_print_mes_num].name,name);

     strcpy(m_print_mes[m_print_mes_num].mes,mes);

    }

    else{                                  //未满，直接添加

         strcpy(m_print_mes[++m_print_mes_num].name,name);

         strcpy(m_print_mes[m_print_mes_num].mes,mes);

    }

    /***********步骤1**************/

    //记录光标位置信息

    printf("\33[s");

    fflush(stdout);

//光标上移到顶端

    printf("\33[25A\n");  

    

    /***********步骤2**************/

    //清空信息显示区域

    for(int i=1;i<=20;i++)

        printf("\33[K\n");

        

    /***********步骤3**************/

//光标上移到顶端  

    printf("\33[21A\n");

    

/***********步骤4**************/

//输出信息

    for(int i=1;i<=m_print_mes_num;i++)

        prinf()       

    //如果信息不到6个，则输出换行符占位

    for(int i=1;i<=6- m_print_mes_num ;i++)

        printf("\n\n\n");

        

/***********步骤5**************/

    //光标还原到原来位置

printf("\33[u");

    fflush(stdout);

}

 
```

 

 


 

---

 

### 4. 文件说明

 
| 文件名              |      功能描述      | 
| -------------    | :----------------------------|
| mian.cpp      |socket启动及链接，开启线程             |
| logic.cpp      | 功能函数：对应了登陆与注册，私聊，群聊，文件传输，好友和群组管理函数 |
| tools.cpp      | 工具函数：求文件大小，发送包，和一些判断函数 | 
|view.cpp|显示的界面函数：菜单显示函数，聊天界面函数，信息查看函数|
|client.h|结构体声明，函数声明，全局变量声明|
|mian.cpp :|socket启动及链接，开启线程，开启多路复用|
|server.cpp|对接收的包按类型进行处理，并把处理结果返回客户端|
|tools.cpp |对包处理时用到的一些功能函数，对存储信息的数组的一些操作|
|Data_access.cpp|用户信息文件的读取与保存，数据库的链接，插入，查询，断开链接|
|md5.cpp|主要是关于md5加密的一些功能函数|
|debug.cpp|调试用时的输出函数|
|Server.h|结构体声明，函数声明，全局变量声明|

 

---

### 5. 异常、错误处理


---

#### 1.输入接受问题：

- 1.程序在选择功能的时候只支持数字，如果进行随意输入，程序不会进行下一步，知道输入符合条件的选项。

- 2.在聊天的时候，程序不支持发送空信息，如果直接按回车键，程序将不会继续执行



---


#### 2.IP地址绑定问题：

    服务端只需自动绑定当前IP即可，而客户端则绑定腾讯云的公网IP

这里把代码做了修改：

```
1. addr.sin_addr.s_addr = htonl(INADDR_ANY);  
```

---

#### 3.包传递问题
```
由于该聊天室全部采用包传递的方式，利用结构体进行传递，而结构体内容如下：

1. typedef struct datas{  

2.     char    send_name[MAX_CHAR];  

3.     char    recv_name[MAX_CHAR];  

4.     int     send_fd;  

5.     int     recv_fd;  

6.     time_t  time;  

7.     char    mes[MAX_CHAR*2];  

8. }DATA;  

9.   

10. typedef struct package{  

11.     int type;  

12.     DATA  data;  

13. }PACK;  
```

这里recv()和send()都传输sizeof(PACK)个字符 

问题：在传输时发现传输总是乱码

原因：在32位操作系统和64位操作系统下long 的字节数不相同，导致sizeof(PACK)不同，导致乱码（time_t实质就是long类型）

解决：后来我把time_t这一项去掉，得到正确结果

注意：***32位机和64位机 字节数的区别***

总结如下：

常用数据类型对应字节数
  可用如sizeof（char),sizeof(char*)等得出

 32位编译器：

      char ：1个字节
      char*（即指针变量）: 4个字节（32位的寻址空间是2^32, 即32个bit，也就是4个字节。同理64位编译器）
      short int : 2个字节
      int：  4个字节
      unsigned int : 4个字节
      float:  4个字节
      double:   8个字节
      long:   4个字节
      long long:  8个字节
      unsigned long:  4个字节

  64位编译器：

      char ：1个字节
      char*(即指针变量): 8个字节
      short int : 2个字节
      int：  4个字节
      unsigned int : 4个字节
      float:  4个字节
      double:   8个字节
      long:   8个字节
      long long:  8个字节
      unsigned long:  8个字节

以上占用字节数其实是针对c/c++语言而言的，对于Java来说由于其JVM具有跨平台性因此java在32位和64位机下基本数据类型占字节数是一致的（这样才能达到跨平台通信）。

---

#### 4.包的重复接受问题

客户端发送一个包，客户端会接受到多个包。

问题原因：在使用epoll多路复用时，使用了while循环，在一个包接受完还没来到及清除缓冲区的情况下，进入第二次循环，就导致了一个包被recv多次，这里猜测，epoll判断客户端有消息是根据缓冲区是否为空判断的。

解决方法：这里我想，没有及时清除缓冲区，那我就在循环中加上usleep，等待其清空完，结果成功解决。  但由于这样太耗费时间，我猜想是因为包太大的缘故，导致清空速达减慢，后来我把包空间减小，果然解决了问题。

注意：在使用epoll时一定要注意其是否即使清空缓冲区，避免多次接受。

---


#### 5.创建线程过多：

问题：在传输文件时，由于每接受一个包就要创建一个线程进行处理，所以这里会出现线程创建过多的情况。

解决方法：为了扩大最大创建线程量，我使用了ulimit -s 命令扩大了线程最多创建数。

在网上查了下，原因如下，Linux系统中每个线程都拥有独立的栈空间，而我的系统上调用ulimit -a看到的结果如下：

ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 20
file size               (blocks, -f) unlimited
pending signals                 (-i) 16382
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1024
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) unlimited
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited

可以看到stack size是8M,  400个线程就需要8*400=3200M，虚拟内存不够用。

 

解决办法有两种：

1.使用ulimit -s 1024*1024命令，将线程栈大小临时设置成1M,经过试验能同时创建2000个线程了。

2.使用pthread_attr_setstacksize在程序中改变线程栈大小。


---


#### 6. 已知存在的问题及改善方案

1.文件传输速度较慢，而且受网速的影响。

2.服务端建立线程数量有时在发送文件时会超过最大限制数量

....

可优化处：文件传输速率

   渐变色，心跳包

 

---


## Analysis/相关文档

[自写聊天室_LinuxC实现(0)——初步代码实现](http://blog.csdn.net/chudongfang2015/article/details/52250340)

[自写聊天室_LinuxC实现(1)——项目部署遇到问题及解决方法](http://blog.csdn.net/chudongfang2015/article/details/52371407)

[自写聊天室_LinuxC实现(2)——服务端模块化优化及流程图](http://blog.csdn.net/chudongfang2015/article/details/52389918)

[自写聊天室_LinuxC实现(3)——客户端模块化优化及流程图](http://blog.csdn.net/chudongfang2015/article/details/52404133)

[ 自写聊天室_LinuxC实现(4)——项目文档](http://blog.csdn.net/chudongfang2015/article/details/52443565)
