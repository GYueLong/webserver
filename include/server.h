/*************************************************************************
	> File Name: Server.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Thu 11 Feb 2021 01:19:02 AM CST
 ************************************************************************/

#ifndef __SERVER_H_
#define __SERVER_H_

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 5

#include "head.h"

class Http;
//class threadpool;
class Server {
public:
    Server(); 
    ~Server(); 

    void init(int argc, char **argv);
    void start();
    Http *users;
    threadpool<Http> *pool = NULL;
    

private:
    int m_port;     //监听端口
    int listenfd;   //监听socket
    int epollfd;
    epoll_event events[MAX_EVENT_NUMBER];
};

#endif
