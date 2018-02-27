#include <mysql/mysql.h> 
#include <gtk/gtk.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>

using namespace std;



#define LOGIN                    1
#define REGISTER                 2
#define FRIEND_SEE               3
#define FRIEND_ADD               4
#define FRIEND_DEL               5
#define GROUP_SEE                6  
#define GROUP_CREATE             7
#define GROUP_JOIN               8
#define GROUP_QIUT               9
#define GROUP_DEL                10
#define CHAT_ONE                 11
#define CHAT_MANY                12
#define FILE_SEND_BEGIN          13
#define FILE_SEND_BEGIN_RP       14
#define FILE_SEND_STOP_RP        15
#define FILE_RECV_RE             16
#define FILE_SEND                17
#define FILE_RECV_BEGIN          18 
#define FILE_RECV_BEGIN_RP       19
#define FILE_RECV_STOP_RP        20
#define FILE_RECV                21
#define FILE_FINI_RP             22
#define MES_RECORD               23
#define EXIT                     -1




#define FILE_STATU_RECV_ING       1
#define FILE_STATU_RECV_STOP      2
#define FILE_STATU_SEND_ING       3
#define FILE_STATU_SEND_FINI      4


#define SIZE_PASS_NAME  30
#define MAX_CHAR        1024
#define EPOLL_MAX       200000
#define LISTENMAX       1000
#define USER_MAX        100  //the user on line
#define GROUP_MAX       50
#define FILE_MAX        50
#define NUM_MAX_DIGIT   10

#define DOWNLINE   0
#define ONLINE     1
#define BUZY       2


// #define LOG
// #define  MYSQL




/********************user infor***********************************/
typedef struct infor_user 
{
    char username[MAX_CHAR];
    char password[MAX_CHAR];
    int  statu;//don't foget to change is to 0 when the server begin
    int  socket_id;
    char friends[USER_MAX][MAX_CHAR];//begin from 1
    int  friends_num;
    char group[GROUP_MAX][MAX_CHAR];  //begin from 1
    char group_num;
}INFO_USER;


INFO_USER  m_infor_user  [USER_MAX];
int        m_user_num; 
/**************************************************/








/*********************group infor***************************/
typedef struct infor_group
{
    char  group_name[MAX_CHAR];
    int   member_num;
    char  member_name[USER_MAX][MAX_CHAR];  //begin from 1
}INFO_GROUP;

INFO_GROUP   m_infor_group  [GROUP_MAX]; //begin from 1
int          m_group_num;






/******************file_infor************************************/

typedef struct file_infor
{
    char  file_name[MAX_CHAR];
    char  file_send_name[MAX_CHAR];
    char  file_recv_name[MAX_CHAR];
    int   file_size;
    int   file_size_now;
    int   flag ;
}INFO_FILE;

INFO_FILE    m_infor_file  [FILE_MAX]; //begin from 1
int          m_file_num;



/*********************************************************/



typedef struct datas{
    char     send_name[MAX_CHAR];
    char     recv_name[MAX_CHAR];
    int      send_fd;
    int      recv_fd;
    time_t   time;
    char     mes[MAX_CHAR*2];
}DATA;

typedef struct package{
    int   type;
    DATA  data;
}PACK;



/*****************send*********************/
PACK m_pack_send   [MAX_CHAR*2];
int  m_send_num;


/**************************************/

typedef struct pthread_parameter
{
    int a;
    char str1[SIZE_PASS_NAME];
    char str2[SIZE_PASS_NAME];
}PTHREAD_PAR;





/**********function***************/
void my_err(const char * err_string,int line);
void signal_close(int i);
void send_pack(PACK *pack_send_pack_t);
void print_send_pack();
void print_infor_group();
void print_infor_user();
void print_infor_file();
int  find_userinfor(char username_t[]);
void login(PACK *recv_pack);
int  judge_usename_same(char username_t[]);
void registe(PACK *recv_pack);
void send_statu(PACK *recv_pack);
void friend_add(PACK *recv_pack);
void del_friend_infor(int id,char friend_name[]); 
void friend_del(PACK *recv_pack);
void send_mes_to_one(PACK *recv_pack);
void group_create(PACK *recv_pack);
void group_join(PACK *recv_pack);
void del_group_from_user(char *username,char *groupname);
void group_qiut(PACK *recv_pack);
void group_del_one(int id);
void group_del(PACK *recv_pack);
int  find_groupinfor(char groupname_t[]);
void send_mes_to_group(PACK *recv_pack);
int  find_fileinfor(char *filename);
void file_recv_begin(PACK *recv_pack);
void file_recv(PACK *recv_pack);
void *pthread_check_file(void *arg);
void send_pack_memcpy_server(int type,char *send_name,char *recv_name,int sockfd1,char *mes);
void *file_send_send(void *file_send_begin_t);
void file_send_begin(PACK *recv_pack);
void file_send_finish(PACK *recv_pack);
void *deal(void *recv_pack_t);
void *serv_send_thread(void *arg);
void init_server_pthread();
int  read_infor();
int  write_infor();










int listenfd,epollfd;//
char* IP = "127.0.0.1";//服务器IP
short PORT = 10222;//端口号
typedef struct sockaddr SA;//类型转换
int log_file_fd;
int user_infor_fd;
int group_infor_fd;
pthread_mutex_t  mutex;
pthread_mutex_t  mutex_recv_file;
pthread_mutex_t  mutex_check_file;


/********************mysql*************************/
MYSQL           mysql;
MYSQL_RES       *res = NULL;
MYSQL_ROW       row;
char            query_str[MAX_CHAR*4] ;
int             rc, fields;
int             rows;


/***********************************************/


void my_err(const char * err_string,int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}




void signal_close(int i)
{
//关闭服务器前 关闭服务器的socket
    #ifdef LOG
        close(log_file_fd);
    #else
    #endif
    
    write_infor(); 
    mysql_free_result(res);
    mysql_close(&mysql);
    printf("服务器已经关闭\n");
    exit(1);
}



void send_pack(PACK *pack_send_pack_t)
{
    pthread_mutex_lock(&mutex);  
    memcpy(&(m_pack_send[m_send_num++]),pack_send_pack_t,sizeof(PACK));
    pthread_mutex_unlock(&mutex);  
}


int read_infor()
{
    INFO_USER read_file_t;
    if((user_infor_fd = open("user_infor.txt",O_RDONLY | O_CREAT , S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return -1;
    } 
    int length ;
    while((length = read(user_infor_fd, &read_file_t, sizeof(INFO_USER))) > 0) 
    {
        read_file_t.statu = DOWNLINE;
        m_infor_user[++m_user_num]  = read_file_t;
    } 
    close(user_infor_fd);
    printf("have read the user_infor.txt.\n");


    INFO_GROUP read_file_group;

    if((group_infor_fd = open("group_infor.txt",O_RDONLY | O_CREAT , S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return -1;
    } 

    while((length = read(group_infor_fd, &read_file_group, sizeof(INFO_GROUP))) > 0) 
    {
        m_infor_group[++m_group_num]  = read_file_group;
    } 
    close(group_infor_fd);
    printf("have read the group_infor.txt.\n");
}



int write_infor()
{
    INFO_USER read_file_t;
    if((user_infor_fd = open("user_infor.txt",O_WRONLY | O_CREAT |O_TRUNC , S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return -1;
    } 
    int length ;
    for(int i= 1 ; i<=m_user_num ;i++)
    {
        if(write(user_infor_fd, m_infor_user+i, sizeof(INFO_USER)) < 0)
        {
            my_err("close",__LINE__);
            return -1;
        }
    }
    close(user_infor_fd);
    printf("have save the user_infor.txt\n");

    


    if((group_infor_fd = open("group_infor.txt",O_WRONLY | O_CREAT |O_TRUNC , S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return -1;
    } 
    for(int i= 1 ; i<=m_group_num ;i++)
    {
        if(write(group_infor_fd, m_infor_group+i, sizeof(INFO_GROUP)) < 0)
        {
            my_err("close",__LINE__);
            return -1;
        }
    }

    close(group_infor_fd);
    printf("have save the group_infor.txt.\n");
}




/***************************debug******************************/

void print_send_pack()
{
    for(int i=1;i<=m_send_num;i++)
    {
        printf("********%d*********\n", i);
        printf("type      :%d\n", m_pack_send[i].type);
        printf("send_name :%s\n", m_pack_send[i].data.send_name);
        printf("recv_name :%s\n", m_pack_send[i].data.recv_name);
        printf("send_fd   :%d\n", m_pack_send[i].data.send_fd);
        printf("recv_fd   :%s\n", m_pack_send[i].data.recv_fd);
        printf("mes       :%s\n", m_pack_send[i].data.mes);
        printf("***********************\n\n\n");
    }
}

void print_infor_group()
{
    for(int i=1;i<=m_group_num;i++)
    {
        printf("\n\n********%d*********\n", i);
        printf("group_name  :%s\n", m_infor_group[i].group_name);
        printf("group_num   :%d\n", m_infor_group[i].member_num);
        for(int j=1 ;j<=m_infor_group[i].member_num;j++)
        printf("*%s\n", m_infor_group[i].member_name[j]);
        printf("***********************\n\n\n");
    }
}


void print_infor_user()
{
    for(int i=1;i<=m_user_num;i++)
    {
        printf("\n**\033[;33m *****user***%d*********\033[0m \n", i);
        printf("user_name  :%s\n", m_infor_user[i].username);
        printf("int  statu :%d\n", m_infor_user[i].statu);
        printf("friends_num:%d\n", m_infor_user[i].friends_num);
        for(int j=1 ;j<=m_infor_user[i].friends_num;j++)
        printf("*%s\n", m_infor_user[i].friends[j]);

        printf("user_num   :%d\n", m_infor_user[i].group_num);
        for(int j=1 ;j<=m_infor_user[i].group_num;j++)
        printf("*%s\n", m_infor_user[i].group[j]);
        printf("***********************\n\n\n");

    }
}

void print_infor_file()
{
    pthread_mutex_lock(&mutex_check_file);  
    for(int i=1;i<=m_file_num;i++)
    {  
        printf("\n\n****file***%d*********\n", i);
        printf("file_name       :%s\n", m_infor_file[i].file_name);
        printf("send_name       :%s\n", m_infor_file[i].file_send_name);
        printf("recv_name       :%s\n", m_infor_file[i].file_recv_name);
        printf("file_size       :%d\n", m_infor_file[i].file_size);
        printf("file_size_now   :%d\n", m_infor_file[i].file_size_now);
        printf("flag            :%d\n", m_infor_file[i].flag);
        printf("***********************\n\n\n");

    }
    pthread_mutex_unlock(&mutex_check_file);  
}


/**********************login*******************************************/

/*void set_user_online(char username_t[])
{
    int i;
    for(i=1;i<=m_send_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
        {
            m_infor_user[i].statu = ONLINE;
            return ;
        }
    }
}*/

/***********************mysql**********************************/
int conect_mysql_init()
{
    

    if (NULL == mysql_init(&mysql)) {               //初始化mysql变量
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        return -1;
    }

    if (NULL == mysql_real_connect(&mysql,         //链接mysql数据库
                "localhost",                       //链接的当地名字
                "root",                            //用户名字
                "root",                            //用户密码
                "happychat",                          //所要链接的数据库
                0,
                NULL,
                0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        return -1;
    }
    printf("1. Connected MySQL successful! \n");
    // query_str = "insert into runoob_tbl values (1,'gaga' ,'justtest', '2015-5-5')";   //插入 SQL 语句
    // rc = mysql_real_query(&mysql, query_str, strlen(query_str));     //对数据库执行 SQL 语句
    // if (0 != rc) {
        // printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        // return -1;
    // }

    /*query_str = "delete from runoob_tbl where runoob_id=2";                          //删除 SQL 语句
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));   //对数据库 执行 SQL 语句
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }*/

    /*query_str = "select * from runoob_tbl";                        //显示 表中数据
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }*/

   /* res = mysql_store_result(&mysql);                             //获取查询结果
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
         return -1;
    }*/

    /*rows = mysql_num_rows(res);                              
    printf("The total rows is: %d\n", rows);

    fields = mysql_num_fields(res);
    printf("The total fields is: %d\n", fields);

    while ((row = mysql_fetch_row(res))) {
        for (i = 0; i < fields; i++) {
            printf("%s\t", row[i]);
        }
        printf("\n");
    }


    mysql_free_result(res);
   */ 
}

















/*************************************************************/
int find_userinfor(char username_t[])
{

    int i;
    if(m_user_num == 0)  return 0;
    for(i=1;i<=m_user_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
            return i;
    }
    if(i == m_user_num+1) 
        return 0;
}




void login(PACK *recv_pack)
{
    int id=0;
    char login_flag[10];

    if((id=find_userinfor(recv_pack->data.send_name)) == 0){//not exit username
        login_flag[0] = '2';
    }
    else if (m_infor_user[id].statu == ONLINE)
    {
        login_flag[0] = '3';

    }
    else if(strcmp(recv_pack->data.mes,m_infor_user[id].password) == 0){//the password is not crrect
        login_flag[0] = '1';
        //change user infor
        
        m_infor_user[id].socket_id = recv_pack->data.send_fd;
        
        printf("\n\n\033[;45m********login**********\033[0m\n");
        printf("\033[;45m*\033[0m %s get online!\n", m_infor_user[id].username);
        printf("\033[;45m*\033[0m statu:    %d\n", m_infor_user[id].statu); 
        printf("\033[;45m*\033[0m socket_id:%d\n\n",m_infor_user[id].socket_id);
    }   
      
    else login_flag[0] = '0';
    
    login_flag[1] = '\0';
    /*printf("%s\n", recv_pack->data.send_name);
    printf("%s\n", recv_pack->data.mes);
    printf("%d\n", m_user_num); */    
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    strcpy(recv_pack->data.mes,login_flag);
    recv_pack->data.recv_fd = recv_pack->data.send_fd;
    recv_pack->data.send_fd = listenfd;
    send_pack(recv_pack);
    usleep(10);
    if(login_flag[0] =='1')
        m_infor_user[id].statu = ONLINE;
    free(recv_pack);
}
/**********************************************************/




/********************registe********************************/
int judge_usename_same(char username_t[])
{
    int i;
    if(m_user_num == 0)  return 1;
    for(i=1;i<=m_user_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
            return 0;
    }
    if(i == m_user_num+1) 
        return 1;
}




void registe(PACK *recv_pack)
{
    char registe_flag[10];
 
    
    if(judge_usename_same(recv_pack->data.send_name)){//the password is not crrect
        registe_flag[0] = '1';
        //add user infor
        m_user_num++;
        strcpy(m_infor_user[m_user_num].username,recv_pack->data.send_name);
        strcpy(m_infor_user[m_user_num].password,recv_pack->data.mes);
        printf("\n\n\033[;44m&&&&function registe&&&&\033[0m \n");
        printf("\033[;44m*\033[0m registe success!\n");
        printf("\033[;44m*\033[0m username:%s\n", m_infor_user[m_user_num].username);
        printf("\033[;44m*\033[0m password:%s\n", m_infor_user[m_user_num].password);
        printf("\033[;44m*\033[0m user_num:%d\n\n", m_user_num);
        m_infor_user[m_user_num].statu = DOWNLINE;
    }
    else registe_flag[0] = '0';
    registe_flag[1] = '\0';
   
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    strcpy(recv_pack->data.mes,registe_flag);
    recv_pack->data.recv_fd = recv_pack->data.send_fd;
    recv_pack->data.send_fd = listenfd;
    send_pack(recv_pack);
    free(recv_pack);

    
}
/**********************************************************/

void send_statu(PACK *recv_pack)
{
    char str[MAX_CHAR];
    char name_t[MAX_CHAR];
    char send_statu_mes[MAX_CHAR*2];
    int id;
    int count = 0;

    memset(send_statu_mes,0,sizeof(send_statu_mes));
    
    id = find_userinfor(recv_pack->data.send_name);
    /*for(int i = 1;i <= m_user_num ;i++)
    {
        if(strcmp(recv_pack->data.send_name,m_infor_user[i].username) == 0)
        {
            id = i;
            break;
        }
    }*/


    /*for(int i=1;i<=m_user_num;i++)
        printf("send_statu:%d,%s\n", i,m_infor_user[i].username);    
    printf("send_statu:id=%d\n", id);*/


    send_statu_mes[count++] = m_infor_user[id].friends_num;


    for(int i=1 ;i <= m_infor_user[id].friends_num ;i++)
    {
        strcpy(name_t,m_infor_user[id].friends[i]);
        printf("send_statufriend:%d %s\n",i,name_t);
        for(int j=1 ;j <= m_user_num ;j++)
        {
            if(strcmp(name_t,m_infor_user[j].username) == 0)
            {
                memset(str,0,sizeof(str));
                if(m_infor_user[j].statu == ONLINE)
                    sprintf(str,"%d%s\0",ONLINE,m_infor_user[j].username);
                else
                    sprintf(str,"%d%s\0",DOWNLINE,m_infor_user[j].username);
                printf("str = %s\n",str);
                for(int k = 0 ;k< SIZE_PASS_NAME;k++)
                {
                    send_statu_mes[k+count] = str[k];
                }
                count += SIZE_PASS_NAME;
            }
        }
    }

    send_statu_mes[count++] = m_infor_user[id].group_num;
    for(int i = 1;i <=  m_infor_user[id].group_num;i++)
    {
        memset(str,0,sizeof(str));
        strcpy(name_t,m_infor_user[id].group[i]);
        sprintf(str,"%s\0",name_t);
        for(int k = 0 ;k< SIZE_PASS_NAME;k++)
        {
            send_statu_mes[k+count] = str[k];
        }
        count += SIZE_PASS_NAME;
    }

    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    memcpy(recv_pack->data.mes,send_statu_mes,MAX_CHAR*2);
    recv_pack->data.recv_fd = recv_pack->data.send_fd;
    recv_pack->data.send_fd = listenfd;

    send_pack(recv_pack);
    free(recv_pack);
}



void friend_add(PACK *recv_pack)
{
    int id_own,id_friend;
    id_own = find_userinfor(recv_pack->data.send_name);
    
    strcpy(m_infor_user[id_own].friends[++(m_infor_user[id_own].friends_num)],recv_pack->data.mes);
    
    id_friend = find_userinfor(recv_pack->data.mes);
    strcpy(m_infor_user[id_friend].friends[++(m_infor_user[id_friend].friends_num)],recv_pack->data.send_name);

    //printf("friend add  m_infor_user[id].friends_num:%d\n", m_infor_user[id].friends_num);
    //printf("friend add  m_infor_user[id].friends_num:%s\n", m_infor_user[id].friends[m_infor_user[id].friends_num]);
    free(recv_pack);
}

void del_friend_infor(int id,char friend_name[]) 
{
    int id_1;
    for(int i = 1 ;i<=m_infor_user[id].friends_num;i++)
    {
        if(strcmp(m_infor_user[id].friends[i],friend_name) == 0)
        {   
            id_1 = i;
            break;
        }

    }
    for(int i = id_1;i<m_infor_user[id].friends_num;i++)
    {
        strcpy(m_infor_user[id].friends[i],m_infor_user[id].friends[i+1]);
    }
    m_infor_user[id].friends_num--;
}







void friend_del(PACK *recv_pack)
{
    int id_own,id_own_1;
    int id_friend,id_friend_1;
    id_own = find_userinfor(recv_pack->data.send_name);
    

    del_friend_infor(id_own,recv_pack->data.mes); 

    
    id_friend = find_userinfor(recv_pack->data.mes);

    del_friend_infor(id_friend,recv_pack->data.send_name); 
    //printf("friend add  m_infor_user[id].friends_num:%d\n", m_infor_user[id].friends_num);
    //printf("friend add  m_infor_user[id].friends_num:%s\n", m_infor_user[id].friends[m_infor_user[id].friends_num]);
    free(recv_pack);

}


void send_mes_to_one(PACK *recv_pack)
{
    // #ifdef MYSQL
        sprintf(query_str, "insert into message_tbl values ('%s','%s' ,'%s','%s')",recv_pack->data.send_name,recv_pack->data.recv_name,recv_pack->data.mes,ctime(&recv_pack->data.time));
        rc = mysql_real_query(&mysql, query_str, strlen(query_str));  
        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return ;
        }
        printf("message get into the mysql!!\n");
    
    // #else
    // #endif
    
    send_pack(recv_pack);
    free(recv_pack);
}









void group_create(PACK *recv_pack)
{
    for(int i=1;i<= m_group_num;i++)
    {
        if(strcmp(m_infor_group[i].group_name,recv_pack->data.mes) == 0)
        {
            strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
            strcpy(recv_pack->data.send_name,"server");
            recv_pack->data.mes[0] = 1;
            send_pack(recv_pack);
            free(recv_pack);
            return ;
        }   
    }
    
    strcpy(m_infor_group[++m_group_num].group_name,recv_pack->data.mes);
    strcpy(m_infor_group[m_group_num].member_name[++m_infor_group[m_group_num].member_num],recv_pack->data.send_name);
    int id=find_userinfor(recv_pack->data.send_name);
    strcpy(m_infor_user[id].group[++m_infor_user[id].group_num],recv_pack->data.mes);


    printf("\n\n\033[;32mcreat group : %s  successfully!\033[0m \n\n", recv_pack->data.mes);
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");

    recv_pack->data.mes[0] = 2;
    send_pack(recv_pack);
    free(recv_pack);
}


void group_join(PACK *recv_pack)
{
    for(int i=1;i<= m_group_num;i++)
    {
        if(strcmp(m_infor_group[i].group_name,recv_pack->data.mes) == 0)
        {
            strcpy(m_infor_group[i].member_name[++m_infor_group[i].member_num],recv_pack->data.send_name);
            int id=find_userinfor(recv_pack->data.send_name);
            strcpy(m_infor_user[id].group[++m_infor_user[id].group_num],recv_pack->data.mes);

            strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
            strcpy(recv_pack->data.send_name,"server");
            recv_pack->data.mes[0] = 2; 
            printf("\n\n\033[;32m %s join group : %s  successfully!\033[0m \n\n",recv_pack->data.recv_name, recv_pack->data.mes);
            print_infor_group();
            print_infor_user();
            send_pack(recv_pack);
            free(recv_pack);
            return ;
        }   
    }
    
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");

    recv_pack->data.mes[0] = 1;
    send_pack(recv_pack);
    free(recv_pack);
}

void del_group_from_user(char *username,char *groupname)
{
    int id = find_userinfor(username);
    for(int i = 1;i<=m_infor_user[id].group_num;i++)
    {
        if(strcmp(m_infor_user[id].group[i],groupname) == 0)
        {
            for(int j = i ;j < m_infor_user[id].group_num ;j++)
            {
                strcpy(m_infor_user[id].group[j],m_infor_user[id].group[j+1]);
            }
            m_infor_user[id].group_num--;
        }
    }
}

void group_qiut(PACK *recv_pack)
{
    del_group_from_user(recv_pack->data.send_name,recv_pack->data.mes);
    for(int i=1 ;i<=m_group_num;i++)
    {
        if (strcmp(recv_pack->data.mes,m_infor_group[i].group_name) == 0)
        {
            for(int j=1;j<=m_infor_group[i].member_num;j++)
            {
                if(strcmp(recv_pack->data.send_name,m_infor_group[i].member_name[j]) == 0)
                {
                    for(int k=j;k <m_infor_group[i].member_num;k++)
                    {
                        strcpy(m_infor_group[i].member_name[k],m_infor_group[i].member_name[k+1]);
                    }
                    m_infor_group[i].member_num--;
                    print_infor_group();
                }
            }
        }
    }
}

void group_del_one(int id)
{
    for(int i=1;i <= m_infor_group[id].member_num;i++)
    {
        del_group_from_user(m_infor_group[id].member_name[i],m_infor_group[id].group_name);
    }

    for(int i = id ;i < m_group_num;i++)
    {
        m_infor_group[i] = m_infor_group[i+1];
    }
    m_group_num--;
}

void group_del(PACK *recv_pack)
{
    for(int i=1;i<=m_group_num;i++)
    {
        if(strcmp(m_infor_group[i].group_name,recv_pack->data.mes) == 0)
        {
            if(strcmp(m_infor_group[i].member_name[1],recv_pack->data.send_name) == 0)
            {
                group_del_one(i);
                recv_pack->data.mes[0] = 2;
                printf("\n\n\033[;32m delete group : %s  successfully!\033[0m \n\n",recv_pack->data.mes);
            }
            else
                recv_pack->data.mes[0] = 1;
        }
    }
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    send_pack(recv_pack);
    //print_infor_group();
    free(recv_pack);

}

int find_groupinfor(char groupname_t[])
{
    int i;
    if(m_group_num == 0)  return 0;
    for(i=1;i<=m_group_num;i++)
    {
        if(strcmp(m_infor_group[i].group_name,groupname_t) == 0)
            return i;
    }
    if(i == m_group_num+1) 
        return 0;
}

void send_mes_to_group(PACK *recv_pack)
{

    int id = find_groupinfor(recv_pack->data.recv_name);
    int len_mes = strlen(recv_pack->data.mes);
    char send_name[SIZE_PASS_NAME];

    for(int i=len_mes;i>=0;i--){
        recv_pack->data.mes[i+SIZE_PASS_NAME] = recv_pack->data.mes[i];
    }
    
    strcpy(send_name ,recv_pack->data.send_name);
    for(int i=0;i<SIZE_PASS_NAME;i++){
        recv_pack->data.mes[i] = recv_pack->data.send_name[i];
    }


    strcpy(recv_pack->data.send_name,recv_pack->data.recv_name);


    for(int i=1; i<=m_infor_group[id].member_num;i++)
    {
        strcpy(recv_pack->data.recv_name,m_infor_group[id].member_name[i]);
        if(strcmp(send_name , m_infor_group[id].member_name[i]) != 0)
        {
           // #ifdef MYSQL
                sprintf(query_str, "insert into message_tbl values ('%s','%s' ,'%s','%s')",recv_pack->data.send_name,recv_pack->data.recv_name,recv_pack->data.mes,ctime(&recv_pack->data.time));
                rc = mysql_real_query(&mysql, query_str, strlen(query_str));     //对数据库执行 SQL 语句
                if (0 != rc) {
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    return ;
                }
                printf("the message get into the mysql!!!\n");
            // #else
            // #endif
            
            send_pack(recv_pack);
        }

    }
    
    free(recv_pack);
}





/*$************************************************************/

int find_fileinfor(char *filename)
{
    for(int i=1 ;i <= m_file_num ;i++)
    {
        if(strcmp(filename , m_infor_file[i].file_name) == 0)
        {
            return i;
        }
    }
    return 0;
}





/*int get_file_size(char *file_name)
{
    FILE *fp = fopen(filename, "r"); 
    fseek(fp, 0, SEEK_END);
    file_size=ftell(fp);//得到文件大小
    
    // printf("file_size=%d",file_size);
    
    send(sockfd,&file_size,sizeof(int),0);// 把文件大小数据发送
    fseek(fp, 0, SEEK_SET);
    close(fp);
}*/





void file_recv_begin(PACK *recv_pack)
{
    int flag = 0;
    int i;
    int file_size_now_t;
    printf("jajahaha  jiji %d\n", m_file_num);

    pthread_mutex_lock(&mutex_check_file);  
    
    for(i=1 ;i<= m_file_num ;i++)
    {
        if(strcmp(m_infor_file[i].file_name,recv_pack->data.mes+NUM_MAX_DIGIT) == 0)
        {
            file_size_now_t = m_infor_file[i].file_size_now;
            flag = 1;
            break;
        } 
    }
    
    if(!flag)
    {
        file_size_now_t = 0;
        strcpy(m_infor_file[++m_file_num].file_name,recv_pack->data.mes+NUM_MAX_DIGIT);
        strcpy(m_infor_file[m_file_num].file_send_name,recv_pack->data.send_name);
        strcpy(m_infor_file[m_file_num].file_recv_name,recv_pack->data.recv_name);
        
        int t=0;
        for(int k=0;k<NUM_MAX_DIGIT;k++)
        {
            if(recv_pack->data.mes[k] == -1)
                break;
            int t1 = 1;
            for(int l=0 ;l<k;l++)
                t1*=10;
            t += (int)(recv_pack->data.mes[k])*t1;
        }
        printf("file_recv_begin t :%d\n", t);
        m_infor_file[m_file_num].file_size = t;
        m_infor_file[m_file_num].file_size_now  = 0;
        m_infor_file[m_file_num].flag = FILE_STATU_RECV_ING;
    }

    pthread_mutex_unlock(&mutex_check_file);  
    


    recv_pack->type = FILE_SEND_BEGIN_RP;
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    

    int digit = 0;
    while(file_size_now_t != 0)
    {   
        recv_pack->data.mes[digit++] = file_size_now_t%10;
        file_size_now_t /= 10;
    }
    recv_pack->data.mes[digit]  = -1;
    


    send_pack(recv_pack);
    //print_infor_file();
    free(recv_pack);
}


void file_recv(PACK *recv_pack)
{
    pthread_mutex_lock(&mutex_recv_file);  
    int fd;
    char file_name[MAX_CHAR];
    char file_path_t[SIZE_PASS_NAME];
    

    int  len = 0;
    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if(recv_pack->data.mes[i] == -1)  
            break;
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        len += (int)recv_pack->data.mes[i]*t1;

    }


    //printf("\033[;33mlen = %d\033[0m \n", len);
    
    strcpy(file_name,recv_pack->data.recv_name);
    //you can creat this file when you get the file_send_begin
    if((fd = open(file_name,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return ;
    }

    if(write(fd,recv_pack->data.mes + NUM_MAX_DIGIT,len) < 0)
        my_err("write",__LINE__);
    // 关闭文件 
    close(fd);
    
    printf("file_name :%s\n", file_name);
    
    int id  =  find_fileinfor(file_name);
    
    m_infor_file[id].file_size_now += len;
    
    free(recv_pack);

    pthread_mutex_unlock(&mutex_recv_file);  
    print_infor_file();
}




/**********************check file statu****************************/

void *pthread_check_file(void *arg)
{
    while(1)
    {
       
        pthread_mutex_lock(&mutex_check_file);  
        for(int i=1 ;i<=m_file_num ;i++)
        {
            
            if(m_infor_file[i].file_size <= m_infor_file[i].file_size_now&&(m_infor_file[i].flag == FILE_STATU_RECV_ING ||m_infor_file[i].flag == FILE_STATU_RECV_STOP))
            {
                char mes_t[MAX_CHAR];
                printf("\033[; 35m**********jijmei********?\033[0m \n");
                int id = find_userinfor(m_infor_file[i].file_recv_name);

                PACK *pthread_check_file_t = (PACK*)malloc(sizeof(PACK));
                memset(pthread_check_file_t, 0,sizeof(PACK));
                pthread_check_file_t->type = FILE_RECV_BEGIN;
                strcpy(pthread_check_file_t->data.send_name ,m_infor_file[i].file_send_name);
                strcpy(pthread_check_file_t->data.recv_name,m_infor_file[i].file_recv_name);
                
                int len = m_infor_file[i].file_size;

                memset(mes_t,0,sizeof(mes_t));
                int digit = 0;
                while(len != 0)
                {   
                    mes_t[digit++] = len%10;
                    len /= 10;
                }
                mes_t[digit]  = -1;
                for(int j=0 ;j< SIZE_PASS_NAME ;j++)
                {
                    mes_t[NUM_MAX_DIGIT+j] = m_infor_file[i].file_name[j];
                }  


                memcpy(pthread_check_file_t->data.mes,mes_t,sizeof(mes_t));  
                send_pack(pthread_check_file_t);
                free(pthread_check_file_t);
                m_infor_file[i].flag = FILE_STATU_SEND_ING;

                
                // m_file_num --;
                // for(int j = i;j<=(m_file_num+1)&&m_file_num;j++)
                // {
                    // m_infor_file[j] = m_infor_file[j+1];
                // }     
                break;
            }


            if(m_infor_file[i].file_size > m_infor_file[i].file_size_now )
            {
                int id = find_userinfor(m_infor_file[i].file_send_name);
                if(m_infor_user[id].statu == DOWNLINE && m_infor_file[i].flag == FILE_STATU_RECV_ING)
                {
                    printf("\033[;35m file_name %s d\033[0m\n", m_infor_file[i].file_name);
                    
                    char mes[MAX_CHAR];
                    memset(mes,0,sizeof(mes));
                    int num = m_infor_file[i].file_size_now;
                    
                    printf("\033[;35m file_sizejj :%d\033[0m\n",num );

                    int digit = 0;
                    while(num != 0)
                    {   
                        mes[digit++] = num%10;
                        num /= 10;
                    }
                    mes[digit] = -1;
                    for(int i=0;i<10;i++)
                        printf("%d\n\n",mes[i]);

                    printf("\033[;35m file_name %s jjj\033[0m\n", m_infor_file[i].file_name);
                   

                    PACK pthread_check_file_t;
                    memset(&pthread_check_file_t, 0,sizeof(PACK));
                    pthread_check_file_t.type = FILE_SEND_STOP_RP;
                    strcpy(pthread_check_file_t.data.send_name ,m_infor_file[i].file_name);
                    strcpy(pthread_check_file_t.data.recv_name,m_infor_file[i].file_send_name);
                    memcpy(pthread_check_file_t.data.mes,mes,sizeof(mes));
                    
                    send_pack(&pthread_check_file_t);
                    printf("jiji$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
                    m_infor_file[i].flag = FILE_STATU_RECV_STOP;
                    break;
                }
            }
            if(m_infor_file[i].flag == FILE_STATU_SEND_FINI)
            {
                unlink(m_infor_file[i].file_name);
                m_file_num --;
                for(int j = i;j<=(m_file_num+1)&&m_file_num;j++)
                {
                    m_infor_file[j] = m_infor_file[j+1];
                } 

            }     
        }
        pthread_mutex_unlock(&mutex_check_file); 
        usleep(10);
    }    
}


void send_pack_memcpy_server(int type,char *send_name,char *recv_name,int sockfd1,char *mes)
{
    PACK pack_send_pack;
    time_t timep;
    pack_send_pack.type = type;
    strcpy(pack_send_pack.data.send_name,send_name);
    strcpy(pack_send_pack.data.recv_name,recv_name);
    memcpy(pack_send_pack.data.mes,mes,MAX_CHAR*2); 
    time(&timep);
    pack_send_pack.data.time = timep;
    printf("sockfd1:%d\n", sockfd1);
    if(send(sockfd1,&pack_send_pack,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}









/*********************deal server to client*****************************/
void *file_send_send(void *file_send_begin_t)
{

    PTHREAD_PAR *file_send_begin = (PTHREAD_PAR *)file_send_begin_t;
    int fd;
    int length;
    int statu;
    int id;
    int sockfd;
    int file_size;
    int begin_location = file_send_begin->a;
    int sum = begin_location;
    char file_name[SIZE_PASS_NAME];
    char recv_name[SIZE_PASS_NAME];
    char mes[MAX_CHAR*2];
    printf("\n\nsending the file.........\n");
    
    strcpy(file_name,file_send_begin->str1);
    strcpy(recv_name,file_send_begin->str2);

    if((fd = open(file_name,O_RDONLY)) == -1)
    {
        my_err("open",__LINE__);
        return NULL;
    }
    
    file_size=lseek(fd, 0, SEEK_END);
    
    lseek(fd ,begin_location ,SEEK_SET);
    printf("file_size %d\n", file_size);
    printf("begin_location%d\n", begin_location);
    bzero(mes, MAX_CHAR*2); 

    // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
    while((length = read(fd  ,mes+NUM_MAX_DIGIT ,MAX_CHAR*2 - NUM_MAX_DIGIT)) > 0) 
    {
        printf("send_::%d\n", sum);

        if(sum == file_size)  
            break;

        id = find_userinfor(recv_name);
        if(m_infor_user[id].statu == DOWNLINE)
        {
            int  file_id   = find_fileinfor(file_name);
            int  file_size = m_infor_file[file_id].file_size;
            char mes_t[MAX_CHAR];
            PACK file_send_stop_t;

            memset(&file_send_stop_t, 0,sizeof(PACK));
            file_send_stop_t.type = FILE_RECV_STOP_RP;
            strcpy(file_send_stop_t.data.send_name ,m_infor_file[file_id].file_send_name);
            strcpy(file_send_stop_t.data.recv_name,m_infor_file[file_id].file_recv_name);
                

            memset(mes_t,0,sizeof(mes_t));
            int digit = 0;
            while(file_size != 0)
            {   
                mes_t[digit++] = file_size%10;
                file_size /= 10;
            }
            mes_t[digit]  = -1;
            for(int j=0 ;j< SIZE_PASS_NAME ;j++)
            {
                mes_t[NUM_MAX_DIGIT+j] = m_infor_file[file_id].file_name[j];
            }  


            memcpy(file_send_stop_t.data.mes,mes_t,sizeof(mes_t));  
            send_pack(&file_send_stop_t);

            free(file_send_begin);
            return NULL ;
        }

        sockfd = m_infor_user[id].socket_id;
        
        int t=length;
        int digit = 0;
        while(t != 0)
        {   
            mes[digit++] = t%10;
            t /= 10;
        }
        mes[digit]  = -1;
        send_pack_memcpy_server(FILE_RECV,file_name,recv_name,sockfd,mes);
        sum += length;
        
        bzero(mes, MAX_CHAR*2); 
        usleep(100000);
    } 
    // 关闭文件 
    close(fd);
    printf("guanle mei ?/**/\n");
    free(file_send_begin);
}





void file_send_begin(PACK *recv_pack)
{
    PTHREAD_PAR *file_send_begin_t;
    file_send_begin_t = (PTHREAD_PAR *)malloc(sizeof(PTHREAD_PAR));
    char recv_name[SIZE_PASS_NAME];
    char file_name[SIZE_PASS_NAME];
    int  begin_location=0;
    strcpy(recv_name,recv_pack->data.send_name);
    strcpy(file_name,recv_pack->data.recv_name);
    printf("file namehahha%s\n", file_name);
    
    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if(recv_pack->data.mes[i] == -1)  
            break;
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        begin_location += (int)recv_pack->data.mes[i]*t1;
    }
    int file_id = find_fileinfor(file_name);

    if(begin_location >= m_infor_file[file_id].file_size)
    {
        m_infor_file[file_id].flag  = FILE_STATU_SEND_FINI;
    }

    file_send_begin_t->a = begin_location;
    //printf("begin_loclation :%d\n",begin_location);
    strcpy(file_send_begin_t->str1,file_name);
    strcpy(file_send_begin_t->str2,recv_name);
    //printf("file name%s\n", file_name);
    file_send_send((void *)file_send_begin_t);
    free(recv_pack);

}


void file_send_finish(PACK *recv_pack)
{   
    printf("^^^^^^^^^^^^^^^^^^^^^^hahah%s\n",recv_pack->data.mes);
    int id = find_fileinfor(recv_pack->data.mes);
    m_infor_file[id].flag = FILE_STATU_SEND_FINI;
    free(recv_pack);
}


void send_record(PACK *recv_pack)
{
    char send_name[MAX_CHAR];
    char recv_name[MAX_CHAR];
    char mes[MAX_CHAR*2];
    PACK *pack_send_record_t = (PACK *)malloc(sizeof(PACK));
    strcpy(send_name,recv_pack->data.send_name);
    strcpy(recv_name,recv_pack->data.mes);
    sprintf(query_str,"select send_name ,recv_name,mes,time from message_tbl where send_name=%s or send_name = %s",send_name,recv_name);            //显示 表中数据
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return ;
    }

    res = mysql_store_result(&mysql);                             //获取查询结果
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
         return ;
    }
    printf("jijmei\n");
    while ((row = mysql_fetch_row(res))) 
    {
        
        pack_send_record_t->type = MES_RECORD;
        strcpy(pack_send_record_t->data.send_name,"server");
        strcpy(pack_send_record_t->data.recv_name,send_name);
        strcpy(pack_send_record_t->data.mes,row[0]);
        strcpy(pack_send_record_t->data.mes+SIZE_PASS_NAME,row[3]);
        strcpy(pack_send_record_t->data.mes+SIZE_PASS_NAME*2,row[2]);
        send_pack(pack_send_record_t);
        usleep(10000);
        
        //for (int i = 0; i < 4; i++) {
          //  printf("%s\t", row[i]);
        //}
        //printf("\n");
    }

    pack_send_record_t->type = MES_RECORD;
    strcpy(pack_send_record_t->data.send_name,"server_end");
    strcpy(pack_send_record_t->data.recv_name,send_name);
    send_pack(pack_send_record_t);

    usleep(10000);
    free(pack_send_record_t);

}



/*****************************************************/


void *deal(void *recv_pack_t)
{
    int i;
    PACK *recv_pack = (PACK *)recv_pack_t;
    printf("\n\n\ndeal function = %d\n", recv_pack->type);
  
    switch(recv_pack->type)
    {
        case LOGIN:
            login(recv_pack);
            break;
        case REGISTER:
            registe(recv_pack);
            break;
        case FRIEND_SEE:
            send_statu(recv_pack);
            break;
        case FRIEND_ADD:
            friend_add(recv_pack);
            break;
        case FRIEND_DEL:
            friend_del(recv_pack);
            break;
        case GROUP_CREATE:
            group_create(recv_pack);
            break;
        case GROUP_JOIN:
            group_join(recv_pack);
            break;
        case GROUP_QIUT:
            group_qiut(recv_pack);
            break;
        case GROUP_DEL:
            group_del(recv_pack);
            break;
        case CHAT_ONE:
            send_mes_to_one(recv_pack);
            break;
        case CHAT_MANY:
            send_mes_to_group(recv_pack);
            break;
        case FILE_SEND_BEGIN:
            file_recv_begin(recv_pack);
            break;
        case FILE_SEND:
            file_recv(recv_pack);
            break;
        case FILE_SEND_BEGIN_RP:
            file_send_begin(recv_pack);
            break;
        case FILE_FINI_RP:
            file_send_finish(recv_pack);
            break;
        case MES_RECORD:
            send_record(recv_pack);
        case EXIT:

            break;       
    }
}





/************************send_thread*****************************/
void *serv_send_thread(void *arg)
{
    int user_statu = DOWNLINE;
    int id_stop;
    int i,recv_fd_t,recv_fd_online;
    while(1)
    {
        pthread_mutex_lock(&mutex); 
        // printf("serv_send_thread:%d\n", m_send_num);
        user_statu = DOWNLINE;
        for(i = m_send_num-1;i>=0;i--)
        {

            for(int j =1; j <= m_user_num ;j++)
            {

                if(strcmp(m_pack_send[i].data.recv_name,m_infor_user[j].username) == 0)
                {
                    user_statu = m_infor_user[j].statu;
                    if(user_statu == ONLINE)
                        recv_fd_online = m_infor_user[j].socket_id;
                    break;
                }
            }

            if(user_statu == ONLINE || m_pack_send[i].type == LOGIN || m_pack_send[i].type == REGISTER)
            {
                if(user_statu == ONLINE)
                    recv_fd_t = recv_fd_online;
                else
                    recv_fd_t = m_pack_send[i].data.recv_fd;
                

                if(send(recv_fd_t,&m_pack_send[i],sizeof(PACK),0) < 0){
                    my_err("send",__LINE__);
                }
                printf("\n\n\033[;42m*****send pack****\033[0m\n");
                printf("\033[;42m*\033[0m type    :%d\n",m_pack_send[i].type);
                printf("\033[;42m*\033[0m from    : %s\n",m_pack_send[i].data.send_name);
                printf("\033[;42m*\033[0m to      : %s\n",m_pack_send[i].data.recv_name);
                printf("\033[;42m*\033[0m mes     : %s\n",m_pack_send[i].data.mes);
                printf("\033[;42m*\033[0m recv_fd : %d\n",m_pack_send[i].data.recv_fd);
                m_send_num-- ;
                printf("\033[;42m*\033[0m pack left Now is:%d\n\n", m_send_num);
                
                for(int j = i;j<=m_send_num&&m_send_num;j++)
                {
                    m_pack_send[j] = m_pack_send[j+1];
                }
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(1);  
    }
}


void init_server_pthread()
{
    printf("\ninit_server_pthread\n");
    pthread_t pid_send,pid_file_check;
    pthread_create(&pid_send,NULL,serv_send_thread,NULL);
    pthread_create(&pid_file_check,NULL,pthread_check_file,NULL);
} 

/****************************************************/





int main()
{
    int n;
    
    pthread_t pid;
    PACK recv_t;
    PACK *recv_pack;
    int err,socketfd,fd_num;
    struct sockaddr_in fromaddr;
    socklen_t len = sizeof(fromaddr);
    struct epoll_event ev, events[LISTENMAX];
    




    read_infor();
    #ifdef LOG
        if((log_file_fd = open("server_log.txt",O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
        {
            my_err("open",__LINE__);
            return 0;
        }
        dup2(log_file_fd , 1);
    #else
    #endif
    
    // #ifdef MYSQL
       conect_mysql_init();
    // #else
    // #endif



    signal(SIGINT,signal_close);//退出CTRL+C
    pthread_mutex_init(&mutex, NULL);  


    printf("服务器开始启动..\n");
    init_server_pthread();
    epollfd = epoll_create(EPOLL_MAX);//生成epoll句柄
    
    listenfd = socket(AF_INET,SOCK_STREAM,0);//启动socket
    if(listenfd == -1){
        perror("创建socket失败");
        printf("服务器启动失败\n");
        exit(-1);
    }

    err = 1;

    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&err,sizeof(int)); 

    ev.data.fd = listenfd;//设置与要处理事件相关的文件描述符
    ev.events = EPOLLIN;//设置要处理的事件类型
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);//注册epoll事件

    //准备网络通信地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if(bind(listenfd,(SA*)&addr,sizeof(addr))==-1){//绑定服务器
        perror("绑定失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("成功绑定\n");

    //设置监听
    if(listen(listenfd,10)==-1){
        perror("设置监听失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("设置监听成功\n");
    printf("初始化服务器成功\n");
    printf("服务器开始服务\n");
    while(1)
    {

        fd_num = epoll_wait(epollfd, events, EPOLL_MAX, 1000);
        for(int i=0; i<fd_num; i++)
        {
            if(events[i].data.fd == listenfd)
            {   

                socketfd = accept(listenfd,(SA*)&fromaddr,&len);
                printf("%d get conect!\n",socketfd);
                ev.data.fd = socketfd;
                ev.events = EPOLLIN;//设置监听事件可写
                //新增套接字
                epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev);
            }
            else if(events[i].events & EPOLLIN)//fd can read
            {

                n = recv(events[i].data.fd,&recv_t,sizeof(PACK),0);//读取数据
                recv_t.data.send_fd = events[i].data.fd;
                
                printf("\n\n\n\033[;31m ***PACK***\033[0m\n");
                printf("\033[;31m*\033[0m type     : %d\n", recv_t.type);
                printf("\033[;31m*\033[0m send_name: %s\n", recv_t.data.send_name);
                printf("\033[;31m*\033[0m recv_name: %s\n",recv_t.data.recv_name);
                printf("\033[;31m*\033[0m mes      : %s\n", recv_t.data.mes);
                printf("\033[;31m*\033[0m send_fd   : %d\n", recv_t.data.send_fd);
                printf("\033[;31m*\033[0m recv_fd   : %d\n", recv_t.data.recv_fd);
                printf("\033[;31m*\033[0m send_pack_num:%d\n", m_send_num);
                printf("\033[;31m*************\033[0m\n\n\n");
                

                if(n < 0)//recv错误
                {     
                    close(events[i].data.fd);
                    perror("recv");
                    continue;
                }
                else if(n == 0)
                {
                 
                    for(int j=1;j<=m_user_num;j++)
                    {
                        if(events[i].data.fd == m_infor_user[j].socket_id)
                        {
                            printf("%s down line!\n",m_infor_user[j].username);
                            m_infor_user[j].statu = DOWNLINE;
                            break;
                        }
                    }   
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);//删除套接字
                    close(events[i].data.fd);
                    print_infor_user();
                    continue;   
                }
                pthread_mutex_lock(&mutex); 
                //print_send_pack();
                
                pthread_mutex_unlock(&mutex);  
                
                //printf("m_file_num1:%d\n", m_file_num);
                //print_infor_file();
               // printf("m_file_num2:%d\n", m_file_num);
                

                recv_pack = (PACK*)malloc(sizeof(PACK));
                memcpy(recv_pack, &recv_t, sizeof(PACK));
                pthread_create(&pid,NULL,deal,(void *)recv_pack);
            }

        }
    }
    return 0;
}
