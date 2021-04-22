/*************************************************************************
	> File Name: util.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Sat 13 Feb 2021 03:01:25 PM CST
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

extern int pipefd[2];

int getArgsint(const char *argname);
char *getArgschar(const char *argname);

int setnonblocking(int fd);
void addfd(int epollfd, int fd, bool enable_et);
void removefd(int epollfd, int fd);
void modfd(int epollfd, int fd, int ev);

//信号处理函数
void sig_handler(int sig);
void addsig(int sig);

//int write_log(char *buf, int length);

#endif
