/*************************************************************************
	> File Name: log.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 08 Mar 2021 09:01:01 PM CST
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sys/time.h>
#include <stdarg.h>

#include "locker.h"

using namespace std;

#define COLOR(no, msg) "\033[" #no "m" msg "\033[0m"
#define COLOR_HL(no, msg) "\033[1;" #no "m" msg "\033[0m"

#define RED(msg) COLOR(31, msg)
#define GREEN(msg) COLOR(32, msg)
#define YELLOW(msg) COLOR(33, msg)

template<class T>
class Msg_que {
public : 
	Msg_que() = default;
	Msg_que(int size) {
		m_max_size = size;
		m_data = new T[size];
		m_cnt = 0;
		m_head = 0;
		m_tail = 0;
	}

	~Msg_que() {
		m_mutex.lock();
		if (m_data != NULL) {
			delete []m_data;
		}
		m_mutex.unlock();
	}

	bool full() {
		bool ret = m_mutex.lock();
		if (m_cnt == m_max_size) {
			m_mutex.unlock();
			return true;
		}
		m_mutex.unlock();
		return false;
	}

	bool empty() {
		m_mutex.lock();
		while (m_cnt == 0) {
			//cout << "empty() wait" << endl;
			if (!m_cond.wait(m_mutex.get())) {
				m_mutex.unlock();
				//cout << "empty() return true" << endl;
				return true;
			}
		}
		m_mutex.unlock();
		return false;
	}

	string front() {
		if (empty()) {
			m_mutex.unlock();
			return "";
		}
		m_mutex.lock();
		string temp = m_data[m_head];
		m_mutex.unlock();
		return temp;
	}

	bool pop() {		
		if (empty()) {
			m_mutex.unlock();
			return false;
		}
		m_mutex.lock();
		m_head++;
		if (m_head == m_max_size) m_head = 0;
		m_cnt--;
		m_mutex.unlock();
		return true;
	}

	bool push(string str) {
		m_mutex.lock();
		if (str == "" || m_cnt == m_max_size)  {
			m_cond.broadcast();
			m_mutex.unlock();
			return false;
		}
		
		m_data[m_tail++] = str;
		if (m_tail == m_max_size) m_tail = 0;
		m_cnt++;

		m_cond.broadcast();
		m_mutex.unlock();
		return true;
	}



private :
	int m_max_size;
	//当前size
	int m_cnt;
	int m_head;
	int m_tail;

	T *m_data;

	locker m_mutex;
	cond m_cond;
};

class Log {
public :
	//队列长度
	static const int QUE_MAX_LENGTH = 1000;
	//日志路径
	char const *LOG_FILE_ROUTE = "./LOG/";
	//后缀
	char const *FILE_TAIL = "log";


	//每条日志长度
	const int log_buf_size = 100;
	
	//单例模式
	static Log *get_instance() {
		static Log instance;
		return &instance;
	}

	static void *log_thread(void *args) {
		Log::get_instance()->async_write_log();
	}

	bool init();
	void write_log(int level, const char *format, ...);
	void flush(void);

private :
	Log() {};
	Msg_que<string> *m_log_queue;
	FILE *m_fp;
	int m_today;//日志日期
	locker m_mutex;
	char *m_buf;
private :
	void async_write_log() {
		//cout << "async_write_log start" << endl;
		string log;
		while (!m_log_queue->empty()) {
			//cout << "wait " << endl;
			m_mutex.lock();
			log = m_log_queue->front();
			//cout << log << endl;
			m_log_queue->pop();
			
			int ret = fwrite((void *)log.c_str(), 1, log.size(), m_fp);
			
			fflush(m_fp);
			//cout << "fputs : " << fputs(log.c_str(), m_fp) << endl;
			//cout << "fwrite ret = " << ret << endl;
			m_mutex.unlock();
		}
	}
};

#define LOG_DEBUG(format, ...) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...)  {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...)  {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif
