void *def(struct Pthread *data)
{
    int conn_fd;
    new_t send_new, temp_new;
    user_t user;
    int ret;
    //int temp_id;
    char temp[20];
    
    conn_fd = data->fd;
    switch (data->new.type)
    {   
        case 1://注册
        case 2://登陆
        case 3://私聊
        case 4://群发
        case 5://查看好友
        case 6://好友管理
        case 7:
            send_new.type = 7;
            strcpy(temp_new.grp.grp_name, data->new.grp.grp_name);
            switch (data->new.flag)
            {
                case 1:
                    look_group(&send_new);
                    send(conn_fd, &send_new, sizeof(new_t), 0);
                    break;
                case 2:
                    if(add_group(&data->new.grp) == 0)
                    {
                        sprintf(send_new.buf, "创建失败：讨论组 %s 已存在",data->new.grp.grp_name);
                        send(conn_fd, &send_new, sizeof(new_t), 0);
                    }
                    else
                    {
                        temp_new.type = 7;
                        sprintf(send_new.buf, "创建讨论组 %s 成功",data->new.grp.grp_name);
                        send(conn_fd, &send_new, sizeof(new_t), 0);
                        
                        strcpy(temp_new.from_name,data->new.grp.make);
                        sprintf(temp_new.buf, "%s 创建了讨论组 %s,您已是其中一员",data->new.grp.make, data->new.grp.grp_name);
                        send_to_more(conn_fd,temp_new);
                    }
                    break;
                case 3:
                    temp_new.type = 7;

                    ret = del_group(data->new.grp.grp_name, data->new.user.name);
                    switch (ret)
                    {
                        case 0:
                            sprintf(send_new.buf, "退出失败：讨论组 %s 不存在",data->new.grp.grp_name);
                            send(conn_fd, &send_new, sizeof(new_t), 0);
                            break;
                        case 1:

                            sprintf(send_new.buf, "解散讨论组 %s 成功",data->new.grp.grp_name);
                            send(conn_fd, &send_new, sizeof(new_t), 0);                         

                            strcpy(temp_new.from_name,data->new.grp.make);
                            sprintf(temp_new.buf, "%s 解散了讨论组 %s",data->new.grp.make, data->new.grp.grp_name);
                            send_to_more(conn_fd,temp_new);
                            
                            break;
                        case 2:
                            sprintf(send_new.buf, "退出讨论组 %s 失败，您并未加入此组",data->new.grp.grp_name);
                            send(conn_fd, &send_new, sizeof(new_t), 0);

                            break;
                        case 3:
                            sprintf(send_new.buf, "退出讨论组 %s 成功",data->new.grp.grp_name);
                            send(conn_fd, &send_new, sizeof(new_t), 0);
    
                            strcpy(temp_new.from_name,data->new.grp.make);
                            sprintf(temp_new.buf, "%s 退出了讨论组 %s",data->new.user.name,data->new.grp.grp_name);
                            send_to_more(conn_fd,temp_new);
                            break;
                        default :
                            break;
                    }
                    break;
            }
            break;
        default:
            break;
            
    }

    free(data);
    pthread_exit(0);
    return NULL;
}

int main()
{
    struct epoll_event ev, events[LISTENMAX];
    int epfd, listen_fd, nfds, sock_fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int n, i, j;
    pthread_t thread;
    new_t new;
    struct Pthread *data;
    user_t user;
    
    epfd = epoll_create(EPOLL_MAX+1);//生成epoll句柄
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);//创建套接字
    i=1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
    ev.data.fd = listen_fd;//设置与要处理事件相关的文件描述符
    ev.events = EPOLLIN;//设置要处理的事件类型
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);//注册epoll事件
        
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   
    addr.sin_port = htons(PORT);

    //绑定套接口
    if(bind(listen_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(1);
    }
    
    listen(listen_fd, LISTENMAX);//转为监听套接字
    
    addr_len = sizeof(struct sockaddr_in);
    
    while(1)
    {
        //等待事件发生
        nfds = epoll_wait(epfd, events, EPOLL_MAX, 1000);
        
        //处理所发生的所有事件
        for(i=0; i<nfds; i++)
        {
            memset(&new, 0, sizeof(new_t));
            
            //连接事件
            if(events[i].data.fd == listen_fd)
            {
                sock_fd = accept(listen_fd, (struct sockaddr*)&addr,&addr_len);
                printf("accept a new client: %s\n",inet_ntoa(addr.sin_addr));
                ev.data.fd = sock_fd;
                ev.events = EPOLLIN;//设置监听事件可写
                //新增套接字
                epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &ev);
            }
            //可读事件
            else if(events[i].events & EPOLLIN)
            {
                n = recv(events[i].data.fd, &new, sizeof(struct News), 0);//读取数据
                
                if(n < 0)//recv错误
                {
                    close(events[i].data.fd);
                    perror("recv");
                    continue;
                }
                else if(n == 0)//下线
                {
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);//删除套接字         
                    for(j=1;j<=2000;j++)
                    {
                        if(f[j] == events[i].data.fd)
                        {
                            break;
                        }
                    }
                    if(j <= 2000)
                    {
                        f[j] = 0;
                        find_user_name(j, &user);
                        printf("%s 下线了！\n",user.name);
                    }

                    close(events[i].data.fd);
                    
                    continue;
                }
                
                data = (struct Pthread*)malloc(sizeof(struct Pthread));
                data->fd = events[i].data.fd;
                memcpy(&data->new, &new, sizeof(new_t));
                pthread_create(&thread, NULL, (void*)def, data);//新开线程去处理事件
            }
        
        }
    }
    
}