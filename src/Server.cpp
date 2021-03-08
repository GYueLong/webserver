/*************************************************************************
	> File Name: Server.cpp
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Thu 11 Feb 2021 01:19:02 AM CST
 ************************************************************************/

#include "../include/Server.h"
#include "../include/conf.h"
#include "../include/http.h"
#include "../include/util.h"
#include <iostream>

using namespace std;


Config config;

Server::Server() {
    m_port = config.getPort();
    cout << "Server() :" << m_port << endl;
}

Server::~Server() {
    
}

void Server::init(int argc, char **argv) {
    if (argc != 1) {
        //getopt
        //更新配置
    }  
    //初始化线程池
    //初始化套接字

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);
 
    epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd, true);
    Http::m_epollfd = epollfd;

    users = new Http[MAX_FD];
    assert(users);

    try {
        pool = new threadpool<Http>;
    } catch (...) {
        return ;
    }

}

void Server::start() {
    while (1) {
        printf("start()\n");
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0) {
            printf("epoll failure\n");
            break;
        }
        //lt(events, ret, epollfd, listenfd); //使用LT模式
        //et(events, ret, epollfd, listenfd); //使用ET模式 
        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                DBG("listenfd\n");
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if (connfd < 0) {
                    printf("error is: %d\n", errno);
                    continue;
                }
                if (Http::m_user_count >= MAX_FD) {
                    //show_error(connfd, "Internal server busy");
                    continue;
                }
                DBG("%d\n", connfd);
                //初始化客户连接
                users[connfd].init(connfd, client_address);
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                DBG("error\n");
                users[sockfd].close_conn();
            } else if (events[i].events & EPOLLIN) {
                DBG("read\n");
                if (users[sockfd].read()) {
                    pool->append(users + sockfd);
                } else {
                    users[sockfd].close_conn();
                }
            } else if (events[i].events & EPOLLOUT) {
                DBG("out\n");
                if (!users[sockfd].write()) {
                    users[sockfd].close_conn();
                }
            }
            else {}
        }
    }
}