#include <mysql/mysql.h> 
#include <gtk/gtk.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <termios.h>  

/*****************same with server**********************/
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


#define SIZE_PASS_NAME   30
#define MAX_PACK_CONTIAN 100
#define MAX_CHAR         1024
#define NUM_MAX_DIGIT    10

#define DOWNLINE   0
#define ONLINE     1
#define BUZY       2



/**************************************************/

typedef struct  friend_info
{
    int statu;
    int mes_num;
    char name[MAX_CHAR];
}FRIEND_INFO; 




typedef struct user_infor{
    char        username    [MAX_CHAR];
    FRIEND_INFO friends     [MAX_CHAR];
    int         friends_num;
    char        group       [MAX_CHAR][MAX_CHAR];
    int         group_num;
}USER_INFOR;

USER_INFOR m_my_infor;







/******************be sure same with server*******************************/
typedef struct datas{
    char    send_name[MAX_CHAR];
    char    recv_name[MAX_CHAR];
    int     send_fd;
    int     recv_fd;
    time_t  time;
    char    mes[MAX_CHAR*2];
}DATA;

typedef struct package{
    int type;
    DATA  data;
}PACK;

typedef struct pthread_parameter
{
    int a;
    int b;
}PTHREAD_PAR;



typedef struct prinit_mes
{
    char name[MAX_CHAR];
    char time[MAX_CHAR];
    char mes [MAX_CHAR];
    
}PRINT_MES;


/************************客户端缓冲区**********************/
/*PACK m_pack_send_friend_see [MAX_CHAR];
PACK m_pack_send_chat_one   [MAX_CHAR];
PACK m_pack_send_chat_many  [MAX_CHAR];
PACK m_pack_send_send_file  [MAX_CHAR];


int m_send_num_friend_see;
int m_send_num_chat_one;
int m_send_num_chat_many;
int n_send_num_send_file;*/
/*****************send*********************/
PACK m_pack_send   [MAX_CHAR];
int  m_send_num;


/*****************recv*********************/
PACK m_pack_recv_friend_see   [MAX_PACK_CONTIAN];
PACK m_pack_recv_chat         [MAX_PACK_CONTIAN];
PACK m_pack_recv_send_file    [MAX_PACK_CONTIAN];
PACK m_pack_recv_file_mes     [MAX_PACK_CONTIAN];
PACK m_pack_recv_file         [MAX_PACK_CONTIAN];

int m_recv_num_friend_see;
int m_recv_num_chat;
int m_recv_num_send_file;
int m_recv_num_file_mes;
int m_recv_num_file;





/****************************************************/


int m_flag_group_create;
int m_flag_group_join ;
int m_flag_group_del  ;
int m_flag_print_mes;
// int window_col;
// int window_row;



/****************************************************/

PRINT_MES m_print_mes[7];
int m_print_mes_num;
/***********************function***********************/
void print_file_mes();
void my_err(const char * err_string,int line);
void init();
void sig_close(int i);
void send_pack(int type,char *send_name,char *recv_name,char *mes);
void send_pack_memcpy(int type,char *send_name,char *recv_name,char *mes);
int  get_choice(char *choice_t);
int  send_login(char username_t[],char password_t[]);
int  login();
int  login_menu();
int  send_registe(char username_t[],char password_t[]);
void registe();
void change_statu(PACK pack_deal_statu_t);
void *deal_statu(void *arg);
void send_file_send(int begin_location,char *file_path);
void *pthread_send_file(void *mes_t);
void *pthread_recv_file(void *par_t);
void init_clien_pthread();
void get_status_mes();
void friends_see();
int  judge_same_friend(char add_friend_t[]);
void add_friend();
void del_friend();
int  judge_same_group(char *group_name);
void group_see();
void send_mes(char mes_recv_name[],int type);
void print_mes(int id);
void *show_mes(void *username);
void send_mes_to_one();
void group_create();
void group_join();
void group_qiut();
void group_del();
void send_mes_to_group();
int  get_file_size(char *file_name);
void send_file();
void file_infor_delete(int id);
void mes_sendfile_fail(int id);
void mes_recv_requir(int id);
void mes_recvfile_fail(int id);
void deal_file_mes(int id);
int  file_mes_box();
int  main_menu();




int sockfd;
char *IP = "192.168.30.3";
short PORT = 10222;
typedef struct sockaddr SA;
pthread_mutex_t  mutex_local_user;

pthread_mutex_t  mutex_recv_file;


void print_file_mes()
{
    for(int i=1 ;i<=5;i++)
    {
        printf("hahahahahaha*****\n");
        printf("%s\n", m_pack_recv_file_mes[i].data.send_name);
        printf("%s",m_pack_recv_file_mes[i].data.recv_name);
        for(int j=0;j<=5;j++)
            printf("%d\n\n",m_pack_recv_file_mes[i].data.mes[j]);

    }
}



void my_err(const char * err_string,int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}

void init()
{
	printf("客户端开始启动\n");
    sockfd = socket(AF_INET,SOCK_STREAM,0);//启动socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if(connect(sockfd,(SA*)&addr,sizeof(addr))==-1){
        perror("无法连接到服务器");
        printf("客户端启动失败\n");
        exit(-1);
    }
    printf("客户端启动成功\n");
}



void sig_close(int i)
{
    //关闭客户端的描述符
    close(sockfd);
    exit(0);
}
/*************************tools******************************/

void send_pack(int type,char *send_name,char *recv_name,char *mes)
{
    PACK pack_send_pack;
    time_t timep;
    pack_send_pack.type = type;
    strcpy(pack_send_pack.data.send_name,send_name);
    strcpy(pack_send_pack.data.recv_name,recv_name);
    strcpy(pack_send_pack.data.mes,mes); 
    time(&timep);
    pack_send_pack.data.time = timep;
    if(send(sockfd,&pack_send_pack,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}



void send_pack_memcpy(int type,char *send_name,char *recv_name,char *mes)
{
    PACK pack_send_pack;
    time_t timep;
    pack_send_pack.type = type;
    strcpy(pack_send_pack.data.send_name,send_name);
    strcpy(pack_send_pack.data.recv_name,recv_name);
    memcpy(pack_send_pack.data.mes,mes,MAX_CHAR*2); 
    time(&timep);
    pack_send_pack.data.time = timep;
    if(send(sockfd,&pack_send_pack,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}



int get_choice(char *choice_t)
{
    int choice =0;
    for(int i=0;i<strlen(choice_t) ;i++)
        if(choice_t[i]<'0' || choice_t[i]>'9')
            return -1;
    for(int i=0;i<strlen(choice_t);i++)
    {
        int t=1;
        for(int j=1;j<strlen(choice_t)-i;j++)
        {
            t *=10;
        }
        choice += t*(int)(choice_t[i] - 48);
    }
    return choice;
}






/***********************mysql**********************************/

















/*************************login************************************/
int send_login(char username_t[],char password_t[])
{
    PACK recv_login_t;
    int login_judge_flag = 0;
    
    send_pack(LOGIN,username_t,"server",password_t);
    
    if(recv(sockfd,&recv_login_t,sizeof(PACK),0) < 0){
        my_err("recv",__LINE__);
    }
    printf("jijimei????\n");
    login_judge_flag = recv_login_t.data.mes[0] - 48;
    return login_judge_flag;
}


int login()
{
    int flag = 0;
    
    int login_flag = 0;
    char username_t [MAX_CHAR];
    char password_t [MAX_CHAR];
    printf("please input the username:\n");
    scanf("%s",username_t);
    printf("please input the password\n");
    scanf("%s",password_t);

    login_flag = send_login(username_t,password_t);
    if(login_flag ==  2){
        printf("\033[;31mthe username is not exit.\033[0m\n");
        return 0;
    }   
    if(login_flag ==  3 ){
        printf("\033[;31mthe user has loged in .\033[0m\n");
        return 0;
    }  
    if(login_flag == 0) {
        printf("\033[;31mthe password is not crrect.\033[0m\n");
        return 0;
    }
    strcpy(m_my_infor.username,username_t);
    printf("\033[;32mload successfully!\033[0m\n");
    return 1;
}


int login_menu()
{
    char choice_t[100];
    int chioce;
    do
    {
        printf("\n\t\t\33[1m\033[;34m*******************************\033[0m\n");
        printf("\t\t\033[;34m*\033[0m        1.login in           \033[;34m*\033[0m \n");
        printf("\t\t\033[;34m*\033[0m        2.register           \033[;34m*\033[0m \n");
        printf("\t\t\033[;34m*\033[0m        0.exit               \033[;34m*\033[0m \n");
        printf("\t\t\033[;34m*******************************\033[0m\n");
        printf("\t\tchoice：");
        scanf("%s",choice_t);
        chioce = get_choice(choice_t);
        switch(chioce)
        {  
            case 1:
                if(login() == 1)
                    return 1;
                break;
            case 2:
                registe();
                break;
            default:
                break;
        }
    }while(chioce!=0);
    return 0;
}
/*******************************************************/







/**************************registe******************************/

int send_registe(char username_t[],char password_t[])
{
    PACK recv_registe_t;
    int send_registe_flag;
    
    send_pack(REGISTER,username_t,"server",password_t);
    
    if(recv(sockfd,&recv_registe_t,sizeof(PACK),0) < 0){
        my_err("recv",__LINE__);
    }
    send_registe_flag = recv_registe_t.data.mes[0] - 48;
    return send_registe_flag;
}




void registe()
{
    int flag = 0;
    flag = REGISTER;
    char username_t[MAX_CHAR];
    char password_t[MAX_CHAR];

   
    
    printf("please input the username you want register:\n");
    scanf("%s",username_t);
    printf("please input the password you want register:\n");
    scanf("%s",password_t);
    if(send_registe(username_t,password_t))
        printf("\033[;32mregiste successfully!\033[0m\n");
    else 
        printf("\033[;31mthe name is used ,please input another one\033[0m\n");
}  


/************************************************************/





/*void *clien_send_thread(void *arg)
{
    int i;
    while(1)
    {
        for(i=0;i<m_send_num;i++)
        {
            if(send(sockfd,m_pack_send+i,sizeof(PACK),0) < 0){
               my_err("send",__LINE__);
            }
        }
        sleep(1);
    }
}*/




void change_statu(PACK pack_deal_statu_t)
{
    int count = 0;
    m_my_infor.friends_num=pack_deal_statu_t.data.mes[count++];

    for(int i=1; i <= m_my_infor.friends_num ;i++)
    {
        for(int j=0;j<SIZE_PASS_NAME;j++)
        {
            if(j == 0)   
                m_my_infor.friends[i].statu = pack_deal_statu_t.data.mes[count+j] - 48;
            else
                m_my_infor.friends[i].name[j-1] = pack_deal_statu_t.data.mes[count+j];
        }
        count += SIZE_PASS_NAME;
    }

    m_my_infor.group_num=pack_deal_statu_t.data.mes[count++];
    for(int i=1 ;i <= m_my_infor.group_num ;i++)
    {
        for(int j=0;j<SIZE_PASS_NAME;j++)
        {
            m_my_infor.group[i][j] = pack_deal_statu_t.data.mes[count+j];
        }
        count += SIZE_PASS_NAME;
    }
}




void *deal_statu(void *arg)
{
    int i;
    printf("function deal_statu:\n");
    while(1)
    {
        pthread_mutex_lock(&mutex_local_user); 
        for(i=1;i<=m_recv_num_friend_see;i++)
        {
            //printf("flag1\n");
            change_statu(m_pack_recv_friend_see[i]);
        }
        m_recv_num_friend_see = 0;
        pthread_mutex_unlock(&mutex_local_user); 
        usleep(1); 
    }
}










void send_file_send(int begin_location,char *file_path)
{
    int fd;
    int length;
    int file_size;
    int sum = begin_location;
    char mes[MAX_CHAR*2];
    printf("\n\nsending the file.........\n");
    

    if((fd = open(file_path,O_RDONLY)) == -1)
    {
        my_err("open",__LINE__);
        return ;
    }
    file_size=lseek(fd, 0, SEEK_END);
    
    printf("file_size=%d",file_size);
    lseek(fd ,begin_location ,SEEK_SET);

    bzero(mes, MAX_CHAR*2); 

    // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
    while((length = read(fd  ,mes+NUM_MAX_DIGIT ,MAX_CHAR*2 - NUM_MAX_DIGIT)) > 0) 
    {
        sum += length;
        printf("length = %d\n", length);
        int digit = 0;
        while(length != 0)
        {   
            mes[digit++] = length%10;
            length /= 10;
        }
        mes[digit]  = -1;
        printf("have sended : %d%%    %d  \n",(int)((double)sum/file_size*100),sum);
        printf("%s\n", file_path);
        send_pack_memcpy(FILE_SEND,m_my_infor.username,file_path,mes);
        
        if(sum == file_size)  
            break;
        bzero(mes, MAX_CHAR*2); 
        usleep(100000);
    } 
    // 关闭文件 
    close(fd);
}







void *pthread_send_file(void *mes_t)
{
    char *mes = (char *)mes_t;
    int begin_location = 0;
    char file_name[MAX_CHAR];
    printf("\033[;31m \nfunction:pthread_send_file\033[0m \n");
    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if(mes[i] == -1)  
            break;
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        begin_location += (int)mes[i]*t1;

    }
    printf("pthread_send_file:%d\n",begin_location);
    strcpy(file_name,mes+NUM_MAX_DIGIT);
    printf("sdfa:%s\n", file_name);
    send_file_send(begin_location,file_name);
}




void *pthread_recv_file(void *par_t)
{
    PTHREAD_PAR * pthread_par  = (PTHREAD_PAR * )par_t;
    int file_size              = pthread_par->a ;
    int begin_location_server  = pthread_par->b;
    int sum                    = begin_location_server; 
    while(1)
    {
        pthread_mutex_lock(&mutex_recv_file); 
        int  fd;
        char file_name[MAX_CHAR];
        for(int i=1;i<=m_recv_num_file ;i++)
        {
            
            int  len = 0;
            for(int j=0 ;j<NUM_MAX_DIGIT ;j++)
            {
                if(m_pack_recv_file[i].data.mes[j] == -1)  
                    break;
                int t1 = 1;
                for(int l=0;l<j;l++)
                    t1*=10;
                len += (int)m_pack_recv_file[i].data.mes[j]*t1;

            }


            //printf("\033[;33mlen = %d\033[0m \n", len);
            
            strcpy(file_name,m_pack_recv_file[i].data.send_name);
            //you can creat this file when you get the file_send_begin
            if((fd = open(file_name,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
            {
                my_err("open",__LINE__);
                return NULL;
            }

            if(write(fd,m_pack_recv_file[i].data.mes + NUM_MAX_DIGIT,len) < 0)
                my_err("write",__LINE__);
            // 关闭文件 
            close(fd);
            sum += len;
            printf("have recved : %d%%    %d  \n",(int)((double)sum/file_size*100),sum);
            
            m_recv_num_file = 0;
            if(sum >= file_size)  
            {
                send_pack(FILE_FINI_RP,m_my_infor.username,"server",file_name);
                return NULL;  
            }
        }

        pthread_mutex_unlock(&mutex_recv_file);
        usleep(10);
    }
    
}















void *clien_recv_thread(void *arg)
{
    int i;
    PACK pack_t;
    pthread_t pid_send_file;
    while(1)
    {
        if(recv(sockfd,&pack_t,sizeof(PACK),0) < 0){
            my_err("recv",__LINE__);
        }
        //for(int i=1;i <= )
        pthread_mutex_lock(&mutex_local_user); 
        
        for(int i=1 ;i<= m_my_infor.friends_num;i++)
        {
            if(strcmp(m_my_infor.friends[i].name,pack_t.data.send_name) == 0)
            {
                m_my_infor.friends[i].mes_num++;
                break;
            }
        }

        switch(pack_t.type)
        {
            printf("clien_recv_thread:%d\n", pack_t.type);
            case FRIEND_SEE:
                m_pack_recv_friend_see[++m_recv_num_friend_see] = pack_t;
                break;
            case GROUP_CREATE:
                m_flag_group_create = pack_t.data.mes[0];
                break;
            case GROUP_JOIN:
                m_flag_group_join   = pack_t.data.mes[0];
                break;
            case GROUP_DEL:
                m_flag_group_del    = pack_t.data.mes[0];
            case CHAT_ONE:
                m_pack_recv_chat[++m_recv_num_chat]             = pack_t;
                break;
            case CHAT_MANY:
                m_pack_recv_chat[++m_recv_num_chat]             = pack_t;
                break;
            //case SEND_FILE:
              //  m_pack_recv_send_file[++m_recv_num_send_file]   = pack_t;
                //break;
            case FILE_SEND_BEGIN_RP:
                 pthread_create(&pid_send_file,NULL,pthread_send_file,(void *)pack_t.data.mes);
                break;
            case FILE_SEND_STOP_RP:
                m_pack_recv_file_mes[++m_recv_num_file_mes]             = pack_t;
                break;
            case FILE_RECV_BEGIN:
                m_pack_recv_file_mes[++m_recv_num_file_mes]             = pack_t;
                break;
            case FILE_RECV:
                pthread_mutex_lock(&mutex_recv_file); 
                m_pack_recv_file[++m_recv_num_file]                     = pack_t;                
                printf("FILE_RECV:%d\n", m_recv_num_file);
                pthread_mutex_unlock(&mutex_recv_file);
                break; 
            case FILE_RECV_STOP_RP:
                m_pack_recv_file_mes[++m_recv_num_file_mes]             = pack_t;
                break;
        }
        pthread_mutex_unlock(&mutex_local_user); 
        usleep(1); 
    }
}


void init_clien_pthread()
{
    pthread_t pid_deal_statu,pid_recv,pid_recv_file;
    pthread_create(&pid_deal_statu,NULL,deal_statu,NULL);

    pthread_create(&pid_recv,NULL,clien_recv_thread,NULL);
} 










/********************friend***************************************/


void get_status_mes()
{
    PACK pack_friend_see;
    pack_friend_see.type = FRIEND_SEE;
    strcpy(pack_friend_see.data.send_name,m_my_infor.username);
    strcpy(pack_friend_see.data.recv_name,"server");
    memset(pack_friend_see.data.mes,0,sizeof(pack_friend_see.data.mes));
    if(send(sockfd,&pack_friend_see,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}



void friends_see()
{
   
    pthread_mutex_lock(&mutex_local_user);
    printf("friends num:%d\n", m_my_infor.friends_num); 
    printf("\n\nthe list of friends:\n");
    for(int i=1 ;i<=m_my_infor.friends_num ;i++)
    {
        printf("m_my_infor.friends[i].statu:%d\n", m_my_infor.friends[i].statu);
        switch(m_my_infor.friends[i].statu)
        {
           case ONLINE:
                printf("ID:%d \033[;32m%s\033[0m[ONLINE] ", i,m_my_infor.friends[i].name);
                if(m_my_infor.friends[i].mes_num)
                    printf("\033[;33m%d messages\033[0m\n", m_my_infor.friends[i].mes_num);
                else 
                    printf("\n");
                break;
           case DOWNLINE:
               printf("ID:%d \033[;31m%s\033[0m[DOWNLINE] ", i,m_my_infor.friends[i].name);
                if(m_my_infor.friends[i].mes_num)
                    printf("\033[;33m%d messages\033[0m\n", m_my_infor.friends[i].mes_num);
                else 
                    printf("\n");
                break;
        }
    }
    pthread_mutex_unlock(&mutex_local_user);  

}


int judge_same_friend(char add_friend_t[])
{
    int i;
    for(i=1;i<=m_my_infor.friends_num;i++)
    {
        if(strcmp(m_my_infor.friends[i].name,add_friend_t) == 0)
            return i;
    }
    return 0;
}



void add_friend()
{
    char add_friend_t[MAX_CHAR];
    
    printf("please input the name of friend you want to add:\n");
    scanf("%s",add_friend_t);
    if(strcmp(add_friend_t,m_my_infor.username) == 0)
    {
        printf("you can't add youself to be your friend!\n");
        return;
    }
    if(judge_same_friend(add_friend_t))
    {
        printf("you already have same friends!\n");
        return ;
    }
    printf("m_my_infor.username:%s\n", m_my_infor.username);
    send_pack(FRIEND_ADD,m_my_infor.username,"server",add_friend_t);
    get_status_mes();
}


void del_friend()
{
    char del_friend_t[MAX_CHAR];
    printf("please input the name of friend you want to delete:\n");
    scanf("%s",del_friend_t);

    if(!judge_same_friend(del_friend_t))
    {
        printf("you don't have this friends on list!\n");
        return ;
    }
    printf("m_my_infor.username:%s\n", m_my_infor.username);
    
    send_pack(FRIEND_DEL,m_my_infor.username,"server",del_friend_t);
    
    get_status_mes();
}

/*************************end_friend************************************/


















/***********************group***********************************/


void show_mes_smart(char *name ,char *time ,char *mes)
{
    int number = 4;
    if(m_print_mes_num == number)  
    {
        for(int i=1;i<=3 ;i++)
            m_print_mes[i] = m_print_mes[i+1];
        strcpy(m_print_mes[number].name,name);
        strcpy(m_print_mes[number].time,time);
        strcpy(m_print_mes[number].mes,mes);
    }
    else{
         strcpy(m_print_mes[++m_print_mes_num].name,name);
         strcpy(m_print_mes[m_print_mes_num].time,time);
         strcpy(m_print_mes[m_print_mes_num].mes,mes);
    }


    printf("\33[s");
    fflush(stdout);

    printf("\33[25A\n");  
    
    for(int i=1;i<=12;i++)
        printf("\33[K\n");

    printf("\33[13A\n");
    for(int i=1;i<=m_print_mes_num;i++)
    {
        printf("\033[40;42m%s\033[0m\t%s",m_print_mes[i].name,m_print_mes[i].time);
        printf("%s\n", m_print_mes[i].mes);
    }
    for(int i=1;i<=4- m_print_mes_num ;i++)
    {
        printf("\n");
        printf("\n\n");

    }
    // printf("\33[11B\n"); 
    printf("\33[u");
    fflush(stdout);
}







int judge_same_group(char *group_name)
{
    int i;
    for(i=1;i<=m_my_infor.group_num;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
            return 1;
    }
    return 0;
}



void group_see()
{
    pthread_mutex_lock(&mutex_local_user); 
    printf("\n\nthe list of group:\n");
    for(int i=1 ;i<=m_my_infor.group_num ;i++)
    {
        printf("ID:%d %s\n", i,m_my_infor.group[i]);
    }
    pthread_mutex_unlock(&mutex_local_user);  
}


void send_mes(char mes_recv_name[],int type)
{
    PACK pack_send_mes;
    char mes[MAX_CHAR];
    time_t timep;
    getchar();
    printf("******************please input****************************\n");
    while(1)
    {   
        time(&timep);
        memset(mes,0,sizeof(mes));
        
        //if(type == CHAT_ONE)
            // printf("%s->",m_my_infor.username);
        fgets(mes,MAX_CHAR,stdin);
        while(mes[0] == 10)
        {
            printf("\33[1A");
            fflush(stdout);
            fgets(mes,MAX_CHAR,stdin);
        }
        if(strcmp(mes,"quit\n") == 0)
            break;
        printf("\33[1A");
        printf("\33[K");
        fflush(stdout);
        show_mes_smart(m_my_infor.username ,ctime(&timep) ,mes);
        
        //printf("\t%s\n%s\n", m_my_infor.username,ctime(&timep),mes);

        send_pack(type,m_my_infor.username,mes_recv_name,mes);
    }
    m_flag_print_mes = EXIT;
}



void print_mes(int id)
{
    char group_print_name[MAX_CHAR];
    memset(group_print_name,0,sizeof(group_print_name));
    if(m_pack_recv_chat[id].type == CHAT_ONE)
    {
        show_mes_smart(m_pack_recv_chat[id].data.send_name,ctime(&m_pack_recv_chat[id].data.time),m_pack_recv_chat[id].data.mes);
    }
    else
    {
        for(int i=0;i<SIZE_PASS_NAME;i++)
        {
            group_print_name[i] = m_pack_recv_chat[id].data.mes[i];
        }
        show_mes_smart( group_print_name,ctime(&m_pack_recv_chat[id].data.time),m_pack_recv_chat[id].data.mes+SIZE_PASS_NAME);
   }
}


//less the button to shutdown
void *show_mes(void *username)
{
    int id;
    char *user_name = (char *)username;
    while(1)
    {
        if (m_flag_print_mes == EXIT)
            break;
        pthread_mutex_lock(&mutex_local_user); 
        id = 0;
        for(int i = 1 ;i <= m_recv_num_chat; i++)
        {
            if(strcmp(m_pack_recv_chat[i].data.send_name,user_name) == 0)
            {
                id = i;
                print_mes(id);
                m_recv_num_chat--;
                for(int j = id; j <= m_recv_num_chat&&m_recv_num_chat ;j++)
                {
                    m_pack_recv_chat[j]  =  m_pack_recv_chat[j+1];
                }
                break;
            }
        }
        
        pthread_mutex_unlock(&mutex_local_user); 
        usleep(1);    
    }
}



void send_mes_to_one()
{
    pthread_t pid;
    int id;
    char mes_recv_name[MAX_CHAR];
    friends_see();//print friend list !

    printf("please input the name you want to chat\n");
    scanf("%s",mes_recv_name);
    if (!(id=judge_same_friend(mes_recv_name)))
    {
        printf("sorry,you don't have the friend named !%s\n",mes_recv_name);
        return ;
    }
    printf("\33[2J \33[30A ***************messages************************");
    printf("\33[23B\n");
    m_flag_print_mes = 1;
    m_my_infor.friends[id].mes_num = 0;
    pthread_create(&pid,NULL,show_mes,(void *)mes_recv_name);
    send_mes(mes_recv_name,CHAT_ONE);
}


void group_create()
{
    char group_name[MAX_CHAR];
    printf("please input the group name you want to create:\n");
    scanf("%s",group_name);
    send_pack(GROUP_CREATE,m_my_infor.username,"server",group_name);
    while(!m_flag_group_create);
    printf("m_flag_group_create=%d\n", m_flag_group_create);
    if(m_flag_group_create == 2) 
        printf("create group successfully!\n");
    else if(m_flag_group_create == 1)
        printf("this group has been created!\n");
    m_flag_group_create = 0;
}



void group_join()
{

    char group_name[MAX_CHAR];
    printf("please input the group name you want to join:\n");
    scanf("%s",group_name);
    
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            printf("you have join this group!\n");
            return ;
        }
    }

    send_pack(GROUP_JOIN,m_my_infor.username,"server",group_name);
    while(!m_flag_group_join);
    printf("m_flag_group_join=%d\n", m_flag_group_join);
    if(m_flag_group_join == 2) 
        printf("join group successfully!\n");
    else if(m_flag_group_join == 1)
        printf("there is no group named %s\n",group_name);
    m_flag_group_join = 0;
}


void group_qiut()
{
    char group_name[MAX_CHAR];
    printf("please input the group name you want to qiut:\n");
    scanf("%s",group_name);
    
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            send_pack(GROUP_QIUT,m_my_infor.username,"server",group_name);
            printf("quit group %s successfully!\n",group_name);
            return ;
        }
    }
    
    printf("you did't join this group!\n");
}


void group_del()
{
    char group_name[MAX_CHAR];
    printf("please input the group name you want to delete:\n");
    scanf("%s",group_name);
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            send_pack(GROUP_DEL,m_my_infor.username,"server",group_name);
            while(!m_flag_group_del);
            printf("m_flag_group_del=%d\n", m_flag_group_del);
            if(m_flag_group_del == 2) 
                printf("delete group successfully!\n");
            else if(m_flag_group_del == 1)
                printf("you isn't the owner of group %s\n",group_name);
            return ;
        }
    }
    
    printf("you did't join this group!\n");
}




void send_mes_to_group()
{
    pthread_t pid;
    char mes_recv_group_name[MAX_CHAR];
    group_see();
    printf("please input the group you want to chat\n");
    scanf("%s",mes_recv_group_name);
    if (!judge_same_group(mes_recv_group_name))
    {
        printf("sorry,you don't have the group named !%s\n",mes_recv_group_name);
        return ;
    }

    m_flag_print_mes = 1;
    printf("\33[2J \33[30A ***************messages************************");
    printf("\33[23B\n");
    pthread_create(&pid,NULL,show_mes,(void *)mes_recv_group_name);
    send_mes(mes_recv_group_name,CHAT_MANY);
    // char a[10],b[10],c[10];
    //scanf("%s %s %s",a,b,c);
    // printf("jijile***(()))\n");
}


/**********************send file********************************/
/***********************file toos********************************/


/*you need creat the file before!*/

int get_file_size(char *file_name)
{
    int fd;
    int len;
    if((fd = open(file_name,O_RDONLY)) == -1)
    {
        my_err("open",__LINE__);
        return 0;
    }
    len = lseek(fd, 0, SEEK_END);
    close(fd);
    return len;
}


/*********************************************************/










void send_file()
{
    char  recv_name[MAX_CHAR];
    char  file_path[MAX_CHAR];
    int   file_size_t;
    char  mes_t[MAX_CHAR];

    printf("please input the friend name:\n");
    scanf("%s",recv_name);

    int id = judge_same_friend(recv_name);
    if(id == 0)
    {
        printf("you don't hava this friend!\n");
        return ;
    }
    printf("please input the path of file you want to send :\n");
    scanf("%s",file_path);
    
    file_size_t = get_file_size(file_path);
    printf("file_size :%d\n", file_size_t);

    if(file_size_t == 0)
    {
        printf("please input creact file path\n");
        return ;
    }

    int digit = 0;
    while(file_size_t != 0)
    {   
        mes_t[digit++] = file_size_t%10;
        file_size_t /= 10;
    }
    mes_t[digit]  = -1;
   

    for(int i=0 ;i< SIZE_PASS_NAME ;i++)
    {
        mes_t[NUM_MAX_DIGIT+i] = file_path[i];
    }
    printf("file_path:%s\n",mes_t+ NUM_MAX_DIGIT);

    send_pack_memcpy(FILE_SEND_BEGIN,m_my_infor.username,recv_name,mes_t);

}


void file_infor_delete(int id)
{
    pthread_mutex_lock(&mutex_local_user); 
    for(int j = id ;j <=m_recv_num_file_mes ;j++)
    {
        m_pack_recv_file_mes[j]  = m_pack_recv_file_mes[j+1];
    }
    m_recv_num_file_mes--;
    pthread_mutex_unlock(&mutex_local_user); 
}



void mes_sendfile_fail(int id)
{
    char chioce[10];
    int begin_location = 0;
    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if( m_pack_recv_file_mes[id].data.mes[i] == -1)  
            break;
        printf("%d\n\n",m_pack_recv_file_mes[id].data.mes[i]);
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        begin_location += (int)m_pack_recv_file_mes[id].data.mes[i] * t1;

    }



    int file_size_t = get_file_size(m_pack_recv_file_mes[id].data.send_name);
    printf("the file %s send failed ,have sended %d%%,do you want send again?\n", m_pack_recv_file_mes[id].data.send_name,(int)((double)begin_location/file_size_t*100));
    printf("y/n :");
    scanf("%s",chioce);
    

    if(chioce[0] != 'Y' && chioce[0] != 'y')
    {
        file_infor_delete(id);
        return ;
    }
    
    
    printf("&&&&&&&\nbegin_location :%d\n",begin_location);


    send_file_send(begin_location,m_pack_recv_file_mes[id].data.send_name);
    file_infor_delete(id);

}


void mes_recv_requir(int id)
{
    pthread_t  pid_recv_file;
    char choice[10];
    int len ;
    int fd;
    char mes_t[MAX_CHAR];
    int file_size = 0;
    char file_name[SIZE_PASS_NAME];
    
    PTHREAD_PAR * par_t = (PTHREAD_PAR *)malloc(sizeof(PTHREAD_PAR));

    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if(m_pack_recv_file_mes[id].data.mes[i] == -1)  
            break;
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        file_size += (int)m_pack_recv_file_mes[id].data.mes[i]*t1;

    }   
    // for(int i=0 ;i<=50;i++)
        // printf("%d\n", m_pack_recv_file_mes[id].data.mes[i]);
    // printf("%s\n", m_pack_recv_file_mes[id].data.mes+NUM_MAX_DIGIT);
    strcpy(file_name,m_pack_recv_file_mes[id].data.mes+NUM_MAX_DIGIT);

    
    printf("  %s send file %s size(%db)to you \n", m_pack_recv_file_mes[id].data.send_name,file_name,file_size);
    printf(" do you want receive it? \n");
    printf("(y/n) :");
    scanf("%s", choice);
    if(choice[0] != 'Y' && choice[0] != 'y')
    {

       /**********************************************/
        file_infor_delete(id);
        /*********************************************/
        
        return ;
    }


    
    if((fd = open(file_name,O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR)) < 0)
    {
        my_err("open",__LINE__);
        return ;
    }
    len = lseek(fd, 0, SEEK_END);
    close(fd);

    par_t->a = file_size;
    par_t->b = len;
    
    int digit = 0;
    while(len != 0)
    {   
        mes_t[digit++] = len%10;
        len /= 10;
    }
    mes_t[digit]  = -1;

    //printf("file_name:%s\n", file_name);
    
    send_pack_memcpy(FILE_SEND_BEGIN_RP ,m_my_infor.username ,file_name ,mes_t);
    

    pthread_create(&pid_recv_file,NULL,pthread_recv_file,(void *)par_t);

    /***************************************************/
   
    file_infor_delete(id);
    /*************************************************/
}



void mes_recvfile_fail(int id)
{
    pthread_t  pid_recv_file;
    char chioce[10];
    int begin_location_server;
    int file_size;
    char file_name[SIZE_PASS_NAME];
    char mes_t[MAX_CHAR];
    PTHREAD_PAR * par_t = (PTHREAD_PAR *)malloc(sizeof(PTHREAD_PAR));

    for(int i=0 ;i<NUM_MAX_DIGIT ;i++)
    {
        if(m_pack_recv_file_mes[id].data.mes[i] == -1)  
            break;
        int t1 = 1;
        for(int l=0;l<i;l++)
            t1*=10;
        file_size += (int)m_pack_recv_file_mes[id].data.mes[i]*t1;

    }   

    strcpy(file_name,m_pack_recv_file_mes[id].data.mes+NUM_MAX_DIGIT);
    
    begin_location_server= get_file_size(file_name);
    

    par_t->a = file_size;
    par_t->b = begin_location_server;
    printf("the file %s recv failed ,have recved %d%%,do you want recv continue?\n", file_name,(int)((double)begin_location_server/file_size*100));
    printf("y/n :");
    scanf("%s",chioce);
    
    

    if(chioce[0] != 'Y' && chioce[0] != 'y')
    {

        /************************************/
        file_infor_delete(id); 
        /************************************/
        
        return ;
    }
    
    
    printf("&&&&&&&\nbegin_location :%d\n",begin_location_server);

    int len = begin_location_server;
    int digit = 0;
    while(len != 0)
    {   
        mes_t[digit++] = len%10;
        len /= 10;
    }
    mes_t[digit]  = -1;




    send_pack_memcpy(FILE_SEND_BEGIN_RP ,m_my_infor.username ,file_name ,mes_t);
    
    pthread_create(&pid_recv_file,NULL,pthread_recv_file,(void *)par_t);

  
  /*****************************************************/  
    file_infor_delete(id);
  /*******************************************************/

}




void deal_file_mes(int id)
{
    if(m_pack_recv_file_mes[id].type == FILE_SEND_STOP_RP)
    {
        mes_sendfile_fail(id);
    }
    else if(m_pack_recv_file_mes[id].type == FILE_RECV_BEGIN)
    {
        mes_recv_requir(id);
    }else if(m_pack_recv_file_mes[id].type == FILE_RECV_STOP_RP)
    {
        mes_recvfile_fail(id);
    }
}









int file_mes_box()
{
    char choice_t[100];
    int chioce;
    do
    {
        get_status_mes();
        printf("pack num_chat:%d\n", m_recv_num_chat);
        printf("\n\t\t************file mes box *************\n");
        for(int i = 1; i <= m_recv_num_file_mes;i++)
        {
            if(m_pack_recv_file_mes[i].type == FILE_SEND_STOP_RP)
                printf("\t\t*        send file %s filed       *\n",m_pack_recv_file_mes[i].data.send_name);
            if(m_pack_recv_file_mes[i].type == FILE_RECV_BEGIN)
                printf("\t\t*        %s send file %s to you       *\n", m_pack_recv_file_mes[i].data.send_name,m_pack_recv_file_mes[i].data.mes+SIZE_PASS_NAME);
            if(m_pack_recv_file_mes[i].type == FILE_RECV_STOP_RP)
                printf("\t\t*         recv file %s filed      *\n", m_pack_recv_file_mes[i].data.mes+NUM_MAX_DIGIT);
        }
        printf("\t\t*        0.exit                 *\n");
        printf("\t\t******************************* *\n");
        printf("\t\tchoice：");
        scanf("%s",choice_t);
        chioce = get_choice(choice_t);
        if(chioce != -1)   
            deal_file_mes(chioce);

    }while(chioce!=0);
    return 0;
}

int mes_record()
{
    char username[MAX_CHAR];
    printf("please input the username that you want see:\n");
    scanf("%s",username);
    printf("the recording :\n");
    send_pack(MES_RECORD,m_my_infor.username,"server",username);

    return 0;
}



int main_menu()
{
    char choice_t[100];
    int chioce;
    do
    {
        get_status_mes();
        printf("pack num_chat:%d\n", m_recv_num_chat);
        printf("\n\t\t*******************************\n");
        printf("\t\t*        1.show   friends       *\n");
        printf("\t\t*        2.add    friends       *\n");
        printf("\t\t*        3.delete friends       *\n");
        printf("\t\t*        4.show   group         *\n");
        printf("\t\t*        5.create group         *\n");
        printf("\t\t*        6.join   group         *\n");
        printf("\t\t*        7.quit   group         *\n");
        printf("\t\t*        8.delete group         *\n");
        printf("\t\t*        9.chat with one        *\n");
        printf("\t\t*        10.chat with many      *\n");
        printf("\t\t*        11.send  file          *\n");
        printf("\t\t*        12.file message box %d  *\n",m_recv_num_file_mes);
        printf("\t\t*        13.mes recording       *\n");
        printf("\t\t*        0.exit                 *\n");
        printf("\t\t******************************* *\n");
        printf("\t\tchoice：");
        scanf("%s",choice_t);
        chioce = get_choice(choice_t);
        switch(chioce)
        {  
            case 1:
                friends_see();
                break;
            case 2:
                add_friend();
                break;
            case 3:
                del_friend();
                break;
            case 4:
                group_see();
                break;
            case 5:
                group_create();
                break;
            case 6:
                group_join();
                break;
            case 7:
                group_qiut();
                break;
            case 8:
                group_del();
                break;
            case 9:
                send_mes_to_one();
                break;
            case 10:
                send_mes_to_group();
                break;
            case 11:
                send_file();
                break;
            case 12:
                file_mes_box();
                break;
            case 13:
                mes_record();
            default:
                break;
        }
    }while(chioce!=0);
    return 0;
}




int main(int argc, char const *argv[])
{
	int flag =0;
    // struct winsize size;  
	
    signal(SIGINT,sig_close);//关闭CTRL+C
    init();//启动并连接服务器
    
    // ioctl(STDIN_FILENO,TIOCGWINSZ,&size); //get size of window
    // window_col = size.ws_col;
    // window_row = size.ws_row;
   
    if(login_menu() == 0)  
        return 0;   
    init_clien_pthread();
    main_menu();
  
    

	return 0;
}
