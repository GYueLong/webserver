/*************************************************************************
	> File Name: http.cpp
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 15 Feb 2021 04:02:43 PM CST
 ************************************************************************/

#include "../include/http.h"

const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file from this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found no this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the requested file.\n";
const char *doc_root = "./var/www";
const char *d_html = "/html";
const char *d_src = "/src";

int Http::m_user_count = 0;
int Http::m_epollfd = -1;

void Http::close_conn(bool real_close) {
	if (real_close && (m_sockfd != -1)) {
		removefd(m_epollfd, m_sockfd);
		m_sockfd = -1;
		m_user_count--;
	}
}

void Http::init(int sockfd, const sockaddr_in &addr) {
	m_sockfd = sockfd;
	m_address = addr;
	int reuse = 1;
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	addfd(m_epollfd, sockfd, true);
	m_user_count++;
	init();
	//printf("用户: %d\n", m_user_count);
}

void Http::init() {
	m_check_state = CHECK_STATE_REQUESTLINE;
	m_linger = false;
	m_method = GET;
	m_url = 0;
	m_version = 0;
	m_content_length = 0;
	m_host = 0;
	m_start_line = 0;
	m_checked_idx = 0;
	m_read_idx = 0;
	m_write_idx = 0;
	f_msg = 0;
	memset(msg, '\0', 50);
	memset(m_read_buf, '\0', READ_BUFFER_SIZE);
	memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
	memset(m_real_file, '\0', FILENAME_LEN);
}

//从状态机
Http::LINE_STATUS Http::parse_line() {
	char temp;
	for (; m_checked_idx < m_read_idx; ++m_checked_idx) {
		temp = m_read_buf[m_checked_idx];
		if (temp == '\r') {
			if ((m_checked_idx + 1) == m_read_idx) {
				return LINE_OPEN;
			} else if (m_read_buf[m_checked_idx + 1] == '\n') {
				m_read_buf[m_checked_idx++] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		} else if (temp == '\n') {
			if ((m_checked_idx > 1) && (m_read_buf[m_checked_idx - 1] == '\r')) {
				m_read_buf[m_checked_idx - 1] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
			}
			return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

//循环读取客户数据，直到无数据可读或者对方关闭连接
bool Http::read() {
	if (m_read_idx >= READ_BUFFER_SIZE) {
		return false;
	}
	int bytes_read = 0;
	while (true) {
		bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
		if (bytes_read == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			return false;
		} else if (bytes_read == 0) {
			return false;
		}
		m_read_idx += bytes_read;
	}
	printf("\n==============\n");
	printf("%s", m_read_buf);
	printf("\n==============\n");
	
	return true;
}

Http::HTTP_CODE Http::parse_request_line(char *text) {
	//解析请求行，首先判断路径，路径非法直接退出
	m_url = strpbrk(text, " \t");
	if (!m_url) {
		return BAD_REQUEST;
	}
	*m_url++ = '\0';
	//解析方法
	char *method = text;
	if (strcasecmp(method, "GET") == 0) {
		m_method = GET;
	} else if (strcasecmp(method, "POST") == 0) {
		m_method = POST;
		f_msg = 1;
	} else {
		return BAD_REQUEST;
	}
	m_url += strspn(m_url, " \t");
	m_version = strpbrk(m_url, " \t");
	if (!m_version) {
		return BAD_REQUEST;
	}
	*m_version++ = '\0';
	m_version += strspn(m_version, " \t");
	if (strcasecmp(m_version, "HTTP/1.1") != 0) {
		return BAD_REQUEST;
	}
	if (strncasecmp(m_url, "http://", 7) == 0) {
		m_url += 7;
		m_url = strchr(m_url, '/');
	}
	if (!m_url || m_url[0] != '/') {
		return BAD_REQUEST;
	}
	m_check_state = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

Http::HTTP_CODE Http::parse_headers(char *text) {
	if (text[0] == '\0') {
		if (m_content_length != 0) {
			m_check_state = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		return GET_REQUEST;
	} else if (strncasecmp(text, "Connection:", 11) == 0) {
		text += 11;
		text += strspn(text, " \t");
		if (strcasecmp(text, "keep-alive") == 0) {
			m_linger = true;
		}
	} else if (strncasecmp(text, "Content-Length:", 15) == 0) {
		text += 15;
		text += strspn(text, " \t");
		m_content_length = atol(text);
	} else if (strncasecmp(text, "Host:", 5) == 0) {
		text += 5;
		text += strspn(text, " \t");
		m_host = text;
	} else {
		printf("oop! unknow header %s\n", text);
	}
	return NO_REQUEST;
}

Http::HTTP_CODE Http::parse_content(char *text) {

	if (f_msg == 1) {
		f_msg = 0;
		strncpy(msg, text + 6, 50);
		strcpy(m_url, "/thanks.html");
		LOG_INFO("%s: %s", inet_ntoa(m_address.sin_addr), msg);
		//printf("msg is %s\n", msg);
		return POST_REQUEST;
	}

	if (m_read_idx >= (m_content_length + m_checked_idx)) {
		text[m_content_length] = '\0';
		return GET_REQUEST;
	}
	return NO_REQUEST;
}



Http::HTTP_CODE Http::process_read() {
	LINE_STATUS line_status = LINE_OK;
	HTTP_CODE ret = NO_REQUEST;
	char *text = 0;
	
	
	//DBG("process_read()");
	while (((m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || ((line_status = parse_line()) == LINE_OK)) {
		text = get_line();
		m_start_line = m_checked_idx;
		printf("got 1 http line: %s\n", text);
		//LOG_INFO("process_read()");
		DBG("m_check_state: %d\n", m_check_state);
		switch (m_check_state) {
			case CHECK_STATE_REQUESTLINE: {
				ret = parse_request_line(text);
				if (ret == BAD_REQUEST) {
					DBG("123\n");
					return BAD_REQUEST;
				}
				break;
			}
			case CHECK_STATE_HEADER: {
				ret = parse_headers(text);
				if (ret == BAD_REQUEST) {
					DBG("456\n");
					return BAD_REQUEST;
				} else if (ret == GET_REQUEST) {
					DBG("789\n");
					return do_request();
				}
				break;
			}
			case CHECK_STATE_CONTENT: {
				ret = parse_content(text);
				if (ret == GET_REQUEST || ret == POST_REQUEST) {
					return do_request();
				}
				line_status = LINE_OPEN;
				break;
			}
			default: {
				return INTERNAL_ERROR;
			}
		}
	}
	return NO_REQUEST;
}

Http::HTTP_CODE Http::do_request() {
	strcpy(m_real_file, doc_root);
	int len = strlen(doc_root);
	//默认返回index.html首页面
	if (strcmp(m_url, "/") == 0) {
		strcat(m_url, "index.html");
	}
	DBG("m_url: %s\n", m_url);

	//处理不同资源目录
	if (strstr(m_url, ".html")) {
		char *temp;
		temp = (char *)malloc(sizeof(char) * URL_LEN);
		strcpy(temp, "/html");
		strcat(temp, m_url);
		strcpy(m_url, temp);
	} else if (strcasecmp(m_url, ".jpg") == 0) {
		//先不处理
	}
	DBG("m_url: %s\n", m_url);
	strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
	DBG("m_real_file: %s\n", m_real_file);
	//获取文件状态
	if (stat(m_real_file, &m_file_stat) < 0) {
		//如果没有资源，返回404页面
		strcpy(m_real_file, doc_root);
		strcat(m_real_file, "/html/404.html");
		int fd = open(m_real_file, O_RDONLY);
		m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		close(fd);
		return NO_RESOURCE;
	}
	//判断是否有读权限
	if (!(m_file_stat.st_mode & S_IROTH)) {
		return FORBIDDEN_REQUEST;
	}
	if (S_ISDIR(m_file_stat.st_mode)) {
		return BAD_REQUEST;
	}
	int fd = open(m_real_file, O_RDONLY);
	m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	return FILE_REQUEST;
}

void Http::unmap() {
	if (m_file_address) {
		munmap(m_file_address, m_file_stat.st_size);
		m_file_address = 0;
	}
}

bool Http::mwrite() {
	int temp = 0;
	int bytes_have_send = 0;
	int bytes_to_send = m_write_idx;
	if (bytes_to_send == 0) {
		modfd(m_epollfd, m_sockfd, EPOLLIN);
		init();
		return true;
	}
	while (1) {
		temp = writev(m_sockfd, m_iv, m_iv_count); 
		if (temp <= -1) {
			if (errno == EAGAIN) {
				modfd(m_epollfd, m_sockfd, EPOLLOUT);
				return true;
			}
			unmap();
			return false;
		} 
		bytes_to_send -= temp;
		bytes_have_send += temp;
		if (bytes_to_send <= bytes_have_send) {
			unmap();
			if (m_linger) {
				init();
				modfd(m_epollfd, m_sockfd, EPOLLIN);
				return true;
			} else {
				modfd(m_epollfd, m_sockfd, EPOLLIN);
				return false;
			}
		}
	}
}

bool Http::add_response(const char *format, ...) {
	if (m_write_idx >= WRITE_BUFFER_SIZE) {
		return false;
	}
	va_list arg_list;
	va_start(arg_list, format);
	int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
	if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx)) {
		return false;
	}
	m_write_idx += len;
	va_end(arg_list);
	return true;
}

bool Http::add_status_line(int status, const char *title) {
	return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool Http::add_headers(int content_len) {
	add_content_length(content_len);
	add_linger();
	add_blank_line();
}

bool Http::add_content_length(int content_len) {
	return add_response("Content-Length: %d\r\n", content_len);
}

bool Http::add_linger() {
	return add_response("Connection: %s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool Http::add_blank_line() {
	return add_response("%s", "\r\n");
}

bool Http::add_content(const char *content) {
	return add_response("%s", content);
}

bool Http::process_write(HTTP_CODE ret) {
	switch (ret) {
		case INTERNAL_ERROR: {
			add_status_line(500, error_500_title);
			add_headers(strlen(error_500_form));
			if (!add_content(error_500_form)) {
				return false;
			}
			break;
		}
		case BAD_REQUEST: {
			add_status_line(400, error_400_title);
			add_headers(strlen(error_400_form));
			if (!add_content(error_400_form)) {
				return false;
			}
			break;
		}
		case NO_RESOURCE: {
			add_status_line(404, error_404_title);
			//add_headers(strlen(error_404_form));
			//if (!add_content(error_404_form)) {
			//	return false;
			//}
			//break;
			if (m_file_stat.st_size != 0) {
				add_headers(m_file_stat.st_size);
				m_iv[0].iov_base = m_write_buf;
				m_iv[0].iov_len = m_write_idx;
				m_iv[1].iov_base = m_file_address;
				m_iv[1].iov_len = m_file_stat.st_size;
				m_iv_count = 2;
				return true;
			} else {
				add_headers(strlen(error_404_form));
				if (!add_content(error_404_form)) {
					return false;
				}
			}
		}
		case FORBIDDEN_REQUEST: {
			add_status_line(403, error_403_title);
			add_headers(strlen(error_403_form));
			if (!add_content(error_403_form)) {
				return false;
			}
			break;
		}
		case FILE_REQUEST: {
			add_status_line(200, ok_200_title);
			if (m_file_stat.st_size != 0) {
				add_headers(m_file_stat.st_size);
				m_iv[0].iov_base = m_write_buf;
				m_iv[0].iov_len = m_write_idx;
				m_iv[1].iov_base = m_file_address;
				m_iv[1].iov_len = m_file_stat.st_size;
				m_iv_count = 2;
				return true;
			} else {
				const char *ok_string = "<html><body>hello</body></html>";
				add_headers(strlen(ok_string));
				if (!add_content(ok_string)) {
					return false;
				}
			}
		}
		default: {
			return false;
		}
	}
	m_iv[0].iov_base = m_write_buf;
	m_iv[0].iov_len = m_write_idx;
	m_iv_count = 1;
	return true;
}

void Http::process() {
	HTTP_CODE read_ret = process_read();
	if (read_ret == NO_REQUEST) {
		DBG("no_request\n");
		modfd(m_epollfd, m_sockfd, EPOLLIN);
		return ;
	}
	DBG("read_ret: %d\n", read_ret);
	bool write_ret = process_write(read_ret);
	if (!write_ret) {
		close_conn();
	}
	modfd(m_epollfd, m_sockfd, EPOLLOUT);
}

/*HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE &checkstate, int &read_index, int &start_line) {
	LINE_STATUS linestatus = LINE_OK;
	HTTP_CODE retcode = NO_REQUEST;
	while ((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK) {
		char *temp = buffer + start_line;
		start_line = checked_index;
		switch (checkstate) {
			case CHECK_STATE_REQUESTLINE: {
				retcode = parse_requestline(temp, checkstate);
				if (retcode == BAD_REQUEST) {
					return BAD_REQUEST;
				}
				break;
			}
			case CHECK_STATE_HEADER: {
				retcode = parse_headers(temp);
				if (retcode == BAD_REQUEST) {
					return BAD_REQUEST;
				} else if (retcode == GET_REQUEST) {
					return GET_REQUEST;
				}
				break;
			}
			default: {
				return INTERNAL_ERROR;
			}
		}
	}
	if (linestatus == LINE_OPEN) {
		return NO_REQUEST;
	} else {
		return BAD_REQUEST;
	}
}

LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index) {
	char temp;
	for (; checked_index < read_index; ++checked_index) {
		temp = buffer[checked_index];
		if (temp == '\r') {
			if ((checked_index + 1) == read_index) {
				return LINE_OPEN;
			} else if (buffer[checked_index + 1] == '\n') {
				buffer[checked_index++] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		} else if (temp == '\n') {
			if ((checked_index > 1) && buffer[checked_index - 1] == '\r') {
				buffer[checked_index - 1] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

HTTP_CODE parse_requestline(char *temp, CHECK_STATE &checkstate) {
	char *url = strpbrk(temp, " \t");
	if (!url) {
		return BAD_REQUEST;
	}
	*url++ = '\0';

	char *method = temp;
	if (strncasecmp(method, "GET") == 0) {
		printf("The request method is GET\n");
	} else {
		return BAD_REQUEST;
	}

	url += strspn(url, " \t");
	char *version = strpbrk(url, " \t");
	if (!version) {
		return BAD_REQUEST;
	}
	*version++ = '\0';
	version += strspn(version, " \t");
	//仅支持HTTP/1.1
	if (strncasecmp(version, "HTTP/1.1") != 0) {
		return BAD_REQUEST;
	}
	//检查url是否合法
	if (strncasecmp(url, "http://", 7) == 0) {
		url += 7;
		url = strchr(url, '/');
	}
	if (!url || url[0] != '/') {
		return BAD_REQUEST;
	}
	printf("The request URL is : %s\n", url);
	checkstate = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

HTTP_CODE parse_headers(char *temp) {
	if (temp[0] == '\0') {
		return GET_REQUEST;
	} else if (strncasecmp(temp, "Host:", 5) == 0) {
		temp += 5;
		temp += strspn(temp, " \t");
		printf("the request host is: %s\n", temp);
	} else {
		//其他头部字段不处理
		printf("I can not handle this header\n");
	}
	return NO_REQUEST;
}*/
