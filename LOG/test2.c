/*************************************************************************
	> File Name: test1.c
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Wed 03 Mar 2021 11:18:01 AM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    int fd;
    fd = open("./log.txt", O_RDONLY);
    if (fd == -1) {
        printf("error\n");
        return 0;
    }
    printf("%d\n", fd);
    scanf("%d", &fd);

    return 0;
}
