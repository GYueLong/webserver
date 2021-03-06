/*************************************************************************
	> File Name: util.c
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Sat 13 Feb 2021 03:01:49 PM CST
 ************************************************************************/

#include "../include/head.h"

int getArgsint(const char *argname) {
    FILE *fp = NULL;
    int port = 0;
	char *line = NULL;
    size_t len = 0;
	ssize_t nread;
    if ((fp = fopen("./server.conf", "r")) == NULL) {
        perror("fopen()");
        return -1;
    }
    
	while ((nread = getline(&line, &len, fp)) != -1) {
		//printf("read %zu : %s\n", nread, line);
        if (strncmp(line, argname, 4) == 0) {
            //printf("%s\n", line);
            char *p = (char *)malloc(sizeof(char) * 5);
            strcpy(p, line + 5);
            port = atoi(p);
            return port;
        }    
    }
	fclose(fp);
    return -1;
}

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd, bool enable_et) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}
void modfd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//int write_log(char *buf, int length) {
//    write(pipefd[1], buf, length);
//}

/*void sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
*/

char *stohex(char *dst, int length)
{
    char buf_temp[2048] = {0};
    char *buf;
    buf = (char *)malloc(sizeof(char) * 2048);
    memset(buf, '\0', 2048);
    for (int i = 0, j = 0; i < length; i++) {
        if (dst[i] == '%') {
            buf[j++] = '\\';
            buf[j++] = 'x';
        } else buf[j++] = dst[i];
    }
    printf("util buf: %s\n", buf);
    memset(dst, '\0', length);
    int x = 0;
    unsigned long i;
    while(*buf != '\0')
    {
        if(*buf == '\\') {
            strcpy(buf_temp,buf);
            *buf_temp = '0';
            *(buf_temp + 4) = '\0';
            i = strtoul(buf_temp, NULL, 16);
            dst[x] = i;
            buf += 3;        
        }
        else {
            dst[x] = *buf;            
        }
        x++;
        buf++;     
    }
    dst[x] = '\0';
    return dst;
}
