/*************************************************************************
	> File Name: head.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Fri 01 Jan 2021 09:09:34 PM CST
 ************************************************************************/

#ifndef _HEAD_H
#define _HEAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>

#include "locker.h"
#include "threadpool.h"
#include "http.h"
#include "conf.h"
#include "server.h"
#include "util.h"

#ifdef DEBUG 
#define DBG(format, args...) {\
	printf(format, ##args);\
}
#else
#define DBG(format, ...)
#endif

#endif
