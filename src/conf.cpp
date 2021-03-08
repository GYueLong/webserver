/*************************************************************************
	> File Name: conf.cpp
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Thu 21 Jan 2021 06:39:35 PM CST
 ************************************************************************/

#include "../include/conf.h"
#include "../include/util.h"

Config::Config() {
}

Config::~Config() {
}

int Config::getPort() {
    int port = 0;
    const char *name = "PORT";
    port = getArgsint(name);
    return port;
}
