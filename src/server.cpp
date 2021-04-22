/*************************************************************************
	> File Name: server.cpp
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Thu 11 Feb 2021 01:19:02 AM CST
 ************************************************************************/

#include "../include/server.h"
#include "../include/conf.h"
#include "../include/http.h"
#include "../include/util.h"
#include <iostream>


Config config;

Server::Server() {
    m_port = config.getPort();
    //std::cout << "Server() :" << m_port << std::endl;
}

Server::~Server() {
    
}

void Server::init(int argc, char **argv) {
    //初始化日志系统
    if (!(Log::get_instance()->init())) {
        LOG_ERROR("log system init fail");
        return ;
    }
    LOG_INFO("webserver init");
    
    //初始化配置
    if (argc != 1) {
        //getopt
        //更新配置
    }  
    
    //初始化套接字

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        LOG_ERROR("listenfd fail, errno:%d", errno);
        return ;
    }
    LOG_INFO("create listenfd ok");

    //设置端口重用
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    if (ret < 0) {
        LOG_ERROR("bind fail, errno:%d", errno);
        return ;
    }
    LOG_INFO("bind ok");

    ret = listen(listenfd, 5);
    if (ret < 0) {
        LOG_ERROR("listen fail, errno:%d", errno);
        return ;
    }
    LOG_INFO("listen ok");
 
    epollfd = epoll_create(5);
    if (epollfd < 0) {
        LOG_ERROR("epoll_create fail, errno:%d", errno);
        return ;
    }
    LOG_INFO("epoll_create ok");
    
    addfd(epollfd, listenfd, true);

    Http::m_epollfd = epollfd;

    users = new Http[MAX_FD];
    assert(users);

    //初始化线程池
    try {
        pool = new threadpool<Http>;
    } catch (...) {
        return ;
    }

}

void Server::start() {
    LOG_INFO("server start...");
    while (1) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0) {
            //链接：https://blog.csdn.net/qiaoliang328/article/details/7404032
            if (errno != EINTR) {
                LOG_ERROR("epoll_wait fail, errno:%d", errno);
                break;
            }
        }
        //lt(events, ret, epollfd, listenfd); //使用LT模式
        //et(events, ret, epollfd, listenfd); //使用ET模式 
        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                //LOG_INFO("new user coming");
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if (connfd < 0) {
                    LOG_ERROR("user accept fail, errno:%d", errno);
                    continue;
                }
                LOG_INFO("new user ip:%s connfd is %d", inet_ntoa(client_address.sin_addr), connfd);

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
                if (!users[sockfd].mwrite()) {
                    users[sockfd].close_conn();
                }
            }
            else {}
        }
    }
}