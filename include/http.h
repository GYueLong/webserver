/*************************************************************************
	> File Name: http.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 15 Feb 2021 03:50:36 PM CST
 ************************************************************************/

#ifndef _HTTP_H
#define _HTTP_H

#include "head.h"

//namespace Http {
class Http {
public:
	//文件名的最大长度
	static const int FILENAME_LEN = 200;
	//读缓存区的大小
	static const int READ_BUFFER_SIZE = 2048;
	//写缓存区的大小
	static const int WRITE_BUFFER_SIZE = 1024;
	//url最大长度
	static const int URL_LEN = 20;
	//HTTP请求方法
	enum METHOD {
		GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH
	};
	//主状态机的两种可能状态，分别表示：当前正在分析请求行，当前正在分析头部字段
	enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0, 
					CHECK_STATE_HEADER,
					CHECK_STATE_CONTENT};
	enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, 
				INTERNAL_ERROR, CLOSED_CONNECTION};
	//从状态机的三种可能状态，即行的读取状态，分别表示：读取到一个完整的行，行出错和行数据尚且不完整
	enum LINE_STATUS {LINE_OK = 0, LINE_BAD, LINE_OPEN};

public:
	Http() {}
	~Http() {}

public:
	//初始化新接受的连接
	void init(int sockfd, const sockaddr_in &addr);
	//关闭连接
	void close_conn(bool real_close = true);
	//处理客户请求
	void process();
	//非阻塞读操作
	bool read();
	//非阻塞写操作
	bool write();

private:
	//初始化连接
	void init();
	//解析http请求
	HTTP_CODE process_read();
	//填充HTTP应答
	bool process_write(HTTP_CODE ret);

	//分析http请求
	HTTP_CODE parse_request_line(char *text);
	HTTP_CODE parse_headers(char *text);
	HTTP_CODE parse_content(char *text);
	HTTP_CODE do_request();
	char *get_line() {
		return m_read_buf + m_start_line;
	}
	LINE_STATUS parse_line();

	//填充HTTP请求
	void unmap();
	bool add_response(const char *format, ...);
	bool add_content(const char *content);
	bool add_status_line(int status, const char *title);
	bool add_headers(int content_length);
	bool add_content_length(int content_length);
	bool add_linger();
	bool add_blank_line();

public:
	static int m_epollfd;
	static int m_user_count;

private:
	int m_sockfd;
	sockaddr_in m_address;
	char m_read_buf[READ_BUFFER_SIZE];
	int m_read_idx;
	int m_checked_idx;
	int m_start_line;
	char m_write_buf[WRITE_BUFFER_SIZE];
	int m_write_idx;

	CHECK_STATE m_check_state;
	METHOD m_method;

	char m_real_file[FILENAME_LEN];
	char *m_url;
	char *m_version;
	char *m_host;
	int m_content_length;
	bool m_linger;
	char *m_file_address;
	struct stat m_file_stat;
	struct iovec m_iv[2];
	int m_iv_count;

};
//}
#endif
