/*************************************************************************
	> File Name: log.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 08 Mar 2021 09:01:01 PM CST
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#define COLOR(no, msg) "\033[" #no "m" msg "\033[0m"
#define COLOR_HL(no, msg) "\033[1;" #no "m" msg "\033[0m"

#define RED(msg) COLOR(31, msg)
#define GREEN(msg) COLOR(32, msg)
#define YELLOW(msg) COLOR(33, msg)


class Log {

};


#endif
