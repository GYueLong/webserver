/*************************************************************************
	> File Name: conf.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Thu 21 Jan 2021 06:39:44 PM CST
 ************************************************************************/

#ifndef _CONF_H
#define _CONF_H

#include "head.h"

class Config {
public:
    Config();
    ~Config();
    int getPort();
    FILE *fp;
private:
    int port;
};

#endif
