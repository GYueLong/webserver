/*************************************************************************
	> File Name: test1.c
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Wed 03 Mar 2021 11:18:01 AM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_LEN 1024

int main(void) {
    int fd;
    fd = open("./log.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 0;
    }
    fd_set readfds;
    int ret;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    ret = select(fd + 1, &readfds, NULL, NULL, 0);
    if (ret == -1) {
        perror("select");
        return 1;
    } 

    while (FD_ISSET(fd, &readfds)) {
        char buf[BUF_LEN + 1];
        int len;
        len = read(fd, buf, BUF_LEN);
        printf("%d\n", len);
        if (len == -1) {
            perror("read");
            return 1;
        }
        if (len) {
            buf[len] = '\0';
            printf("read : %s", buf);
        }
    }

    return 0;
}
