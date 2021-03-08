/*************************************************************************
	> File Name: main.c
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Fri 01 Jan 2021 09:10:23 PM CST
 ************************************************************************/

#include "./include/head.h"

int main(int argc, char **argv) {

    pid_t pid;
    if (fork() < 0) {
        perror("fork()");
    } else if (fork() > 0) {
        exit(0);
    }
    //子进程
    setsid();
    close(1);


    Server server;
    server.init(argc, argv);
    server.start();

    return 0;
}
