/*************************************************************************
	> File Name: log.cpp
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 08 Mar 2021 09:00:04 PM CST
 ************************************************************************/

#include "../include/log.h"

bool Log::init() {
	m_log_queue = new Msg_que<string>(QUE_MAX_LENGTH);

	pthread_t tid;
	pthread_create(&tid, NULL, log_thread, NULL);

	time_t t = time(NULL);
	struct tm *sys_tm = localtime(&t);
	struct tm my_tm = *sys_tm;

	m_buf = new char[log_buf_size];
    memset(m_buf, '\0', log_buf_size);

	char log_full_name[256] = {0};

	snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", LOG_FILE_ROUTE, my_tm.tm_year + 1900, my_tm.tm_mon +1, my_tm.tm_mday, FILE_TAIL);

	m_today = my_tm.tm_mday;

	m_fp = fopen(log_full_name, "a");

	//cout << "fopen m_fp: " << m_fp << endl;

	if (m_fp == NULL) {
		return false;
	}
	return true;
}

void Log::write_log(int level, const char *format, ...) {
	//printf("%s\n", "write_log");
	struct timeval now = {0, 0};
	gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
	char s[16] = {0};
    switch (level)
    {
    case 0:
        strcpy(s, "[debug]:");
        break;
    case 1:
        strcpy(s, "[info]:");
        break;
    case 2:
        strcpy(s, "[warn]:");
        break;
    case 3:
        strcpy(s, "[erro]:");
        break;
    default:
        strcpy(s, "[info]:");
        break;
    }

	m_mutex.lock();

	if (m_today != my_tm.tm_mday) {
		char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};

		snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
		snprintf(new_log, 255, "%s%s%s", LOG_FILE_ROUTE, tail, FILE_TAIL);
        m_today = my_tm.tm_mday;

		m_fp = fopen(new_log, "a");
	}
	m_mutex.unlock();

	va_list valst;
    va_start(valst, format);

	string log_str;
	m_mutex.lock();

    //写入的具体时间内容格式
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
	int m = vsnprintf(m_buf + n, log_buf_size - 1, format, valst);
	m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    log_str = m_buf;
	
	m_mutex.unlock();

	//std::cout << "log_str" << log_str << std::endl;

	if (!m_log_queue->full()) {
		m_log_queue->push(log_str);
	}
	va_end(valst);

}

void Log::flush(void)
{
    m_mutex.lock();
    //强制刷新写入流缓冲区
    fflush(m_fp);
    m_mutex.unlock();
}
