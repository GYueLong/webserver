/*************************************************************************
	> File Name: locker.h
	> Author: gyl
	> Mail: gyldeyx@qq.com
	> Created Time: Mon 15 Feb 2021 11:40:02 PM CST
 ************************************************************************/

#ifndef _LOCKER_H
#define _LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

//封装信号量的类
class sem {
public:
	sem() {
		if (sem_init(&m_sem, 0, 0) != 0) {
			throw std::exception();
		}
	}
	~sem() {
		sem_destroy(&m_sem);
	}
	bool wait() {
		return sem_wait(&m_sem) == 0;
	}
	bool post() {
		return sem_post(&m_sem) == 0;
	}
private:
	sem_t m_sem;
};

class locker {
public:
	locker() {
		if (pthread_mutex_init(&m_mutex, NULL) != 0) {
			throw std::exception();
		}
	}
	~locker() {
		pthread_mutex_destroy(&m_mutex);
	}
	bool lock() {
		bool ret = (pthread_mutex_lock(&m_mutex) == 0);
		return ret;
	}
	bool unlock() {
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
	pthread_mutex_t *get()
    {
        return &m_mutex;
    }
private:
	pthread_mutex_t m_mutex;
};

/*class cond {
public:
	cond() {
		if (pthread_mutex_init(&m_mutex, NULL) != 0) {
			throw std::exception();
		}
		if (pthread_cond_init(&m_cond, NULL) != 0) {
			pthread_mutex_destroy(&m_mutex);
			throw std::exception();
		}
	}
	~cond() {
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy(&m_cond);
	}
	bool wait() {
		int ret = 0;
		pthread_mutex_lock(&m_mutex);
		ret = pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);
		return ret == 0;
	}
	bool signal() {
		return pthread_cond_signal(&m_cond) == 0;
	}
private:
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};*/

class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif
